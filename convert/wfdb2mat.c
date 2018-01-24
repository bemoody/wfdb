/* file: wfdb2mat.c	G. Moody	26 February 2009
			Last revised:	 23 January 2018
-------------------------------------------------------------------------------
wfdb2mat: Convert (all or part of) a WFDB signal file to Matlab .mat format
Copyright (C) 2009-2013 George B. Moody

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA.

You may contact the author by e-mail (george@mit.edu) or postal mail
(MIT Room E25-505A, Cambridge, MA 02139 USA).  For updates to this software,
please visit PhysioNet (http://www.physionet.org/).
_______________________________________________________________________________

This program converts the signals of any PhysioBank record (or one in any
compatible format) into a .mat file that can be read directly using any version
of Matlab, and a short text file containing information about the signals
(names, gains, baselines, units, sampling frequency, and start time/date if
known).  If the input record name is REC, the output files are RECm.mat and
RECm.hea.  The output files can also be read by any WFDB application as record
RECm.

This program does not convert annotation files; for that task, 'rdann' is
recommended.

The output .mat file contains a single matrix named 'val' containing raw
(unshifted, unscaled) samples from the selected record.  Using various options,
you can select any time interval within a record, or any subset of the signals,
which can be rearranged as desired within the rows of the matrix.  Since .mat
files are written in column-major order (i.e., all of column n precedes all of
column n+1), each vector of samples is written as a column rather than as a
row, so that the column number in the .mat file equals the sample number in the
input record (minus however many samples were skipped at the beginning of the
record, as specified using the -f option).  If this seems odd, transpose your
matrix after reading it!

This program writes version 5 MAT-file format output files, as documented in
http://www.mathworks.com/access/helpdesk/help/pdf_doc/matlab/matfile_format.pdf
The samples are written as 32-bit signed integers (mattype=20 below) in
little-endian format if the record contains any format 24 or format 32 signals,
as 8-bit unsigned integers (mattype=50) if the record contains only format 80
signals, or as 16-bit signed integers in little-endian format (mattype=30)
otherwise.

The maximum size of the output variable is 2^31 bytes. wfdb2mat from versions
10.5.24 and earlier of the WFDB software package writes version 4 MAT files
which have the additional constraint of 100,000,000 elements per variable.

The output files (recordm.mat + recordm.hea) are still WFDB-compatible, given
the .hea file constructed by this program.

Example:

To convert record mitdb/200, use this command:
    wfdb2mat -r mitdb/200

This works even if the input files have not been downloaded;  in this case,
wfdb2mat reads them directly from the PhysioNet server.

The output files are mitdb/200m.mat and mitdb/200m.hea.  Note that if a
subdirectory named 'mitdb' did not exist already, it would be created by
wfdb2mat.

*/

#include <stdio.h>
#include <wfdb/wfdb.h>

/* Output .mat data storage types (values of mattype), defined by the
   .mat format specification.  These variables are for the sub4 tag
   data STORAGE type. */
#define MAT8 2	/* 8 bits per sample unsigned */
#define MAT16 3	/* 16 bits per sample signed */
#define	MAT32 5	/* 32 bits per sample signed */

char *pname;

main(argc, argv)
int argc;
char *argv[];
{
    char *matname, *orec, *p, *q, *record = NULL, *search = NULL, *prog_name();

    /* The entire file is composed of:
       - 128 byte descriptive text
       - 8 byte master tag. 4 bytes indicate data type = matrix, 4
         bytes indicate data size.
       - 4 subelements. Each subelement has a 4 byte tag giving the
         data type of the elements, a 4 byte tag giving the subelement
         size, and the subelement's actual content.
         - Subelement 1: array flags (8 + 8 bytes)
         - Subelement 2: array dimension (8 + 8 bytes)
         - Subelement 3: array name (8 + 8 bytes)
         - Subelement 4: array content (8 + ?? bytes)
    */
    static char prolog[192];  /* 128 byte descriptive text
				 + 64 bytes of fixed length content */
    /* mastertype=matrix, sub1type=UINT32, sub2type=INT32, sub3type=INT8
       fieldversion=0x0100 indicating mat file */
    int highres = 0, i, isiglist, mattype, nisig, nosig = 0, pflag = 0,
	mastertype = 14, sub1type = 6, sub2type = 5, sub3type = 1, sub4type,
	fieldversion = 256, s, sfname = 0, *sig = NULL, stat = 0, vflag = 0,
	sub1class = 6, nbytesperelement, wfdbtype, offset;

    /* nbytesmaster is the value specified in the master tag
       representing the size of all following content.
       nbytesofdata gives the number of bytes of signal data.
       lremain stores the number of bytes to pad to make the file size
       a multiple of 8.
       max_length is the maximum permissible signal length. */
    unsigned long nbytesmaster, nbytesofdata, lremain, max_length;

    WFDB_Frequency freq;
    WFDB_Sample *vi, *vo;
    WFDB_Siginfo *si, *so;
    WFDB_Time from = 0L, maxl = 0L, t, to = 0L;
    void help();

    pname = prog_name(argv[0]);
    for (i = 1 ; i < argc; i++) {
	if (*argv[i] == '-') switch (*(argv[i]+1)) {
	  case 'f':	/* starting time */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: time must follow -f\n", pname);
		exit(1);
	    }
	    from = i;
	    break;
	  case 'h':	/* help requested */
	    help();
	    exit(0);
	    break;
	  case 'H':	/* select high-resolution mode */
	    highres = 1;
	    break;
	  case 'l':	/* maximum length of output follows */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: max output length must follow -l\n",
			      pname);
		exit(1);
	    }
	    maxl = i;
	    break;
	  case 'r':	/* record name */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: record name must follow -r\n",
			      pname);
		exit(1);
	    }
	    record = argv[i];
	    break;
	  case 's':	/* signal list follows */
	    isiglist = i+1; /* index of first argument containing a signal # */
	    while (i+1 < argc && *argv[i+1] != '-') {
		i++;
		nosig++;	/* number of elements in signal list */
	    }
	    if (nosig == 0) {
		(void)fprintf(stderr, "%s: signal list must follow -s\n",
			pname);
		exit(1);
	    }
	    break;
	  case 'S':	/* search for valid sample of specified signal */
	    if (++i >= argc) {
		(void)fprintf(stderr,
			      "%s: signal name or number must follow -S\n",
			      pname);
		exit(1);
	    }
	    search = argv[i];
	    break;
	      
	  case 't':	/* end time */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: time must follow -t\n",pname);
		exit(1);
	    }
	    to = i;
	    break;
	  default:
	    (void)fprintf(stderr, "%s: unrecognized option %s\n", pname,
			  argv[i]);
	    exit(1);
	}
	else {
	    (void)fprintf(stderr, "%s: unrecognized argument %s\n", pname,
			  argv[i]);
	    exit(1);
	}
    }
    if (record == NULL) {
	help();
	exit(1);
    }
    if ((nisig = isigopen(record, NULL, 0)) <= 0) exit(2);
    SUALLOC(si, nisig, sizeof(WFDB_Siginfo));
    SUALLOC(vi, nisig, sizeof(WFDB_Sample));
    if ((nisig = isigopen(record, si, nisig)) <= 0)
	exit(2);
    for (i = 0; i < nisig; i++)
	if (si[i].gain == 0.0) si[i].gain = WFDB_DEFGAIN;
    if (highres)
        setgvmode(WFDB_HIGHRES);
    freq = sampfreq(NULL);
    if (from > 0L && (from = strtim(argv[from])) < 0L)
	from = -from;
    if (isigsettime(from) < 0)
	exit(2);
    if (to > 0L && (to = strtim(argv[to])) < 0L)
	to = -to;

    if (nosig) {	      /* convert samples only from specified signals */
	SUALLOC(so, nosig, sizeof(WFDB_Siginfo));
	SUALLOC(vo, nosig, sizeof(WFDB_Sample));
	SUALLOC(sig, nosig, sizeof(int));
	for (i = 0; i < nosig; i++) {
	    if ((s = findsig(argv[isiglist+i])) < 0) {
		(void)fprintf(stderr, "%s: can't read signal '%s'\n", pname,
			      argv[isiglist+i]);
		exit(2);
	    }
	    sig[i] = s;
	}
    }
    else {			/* convert samples from all signals */
	nosig = nisig;
	SUALLOC(so, nosig, sizeof(WFDB_Siginfo));
	SUALLOC(vo, nosig, sizeof(WFDB_Sample));
	SUALLOC(sig, nosig, sizeof(int));
	for (i = 0; i < nosig; i++)
	    sig[i] = i;
    }

    /* Reset 'from' if a search was requested. */
    if (search &&
	((s = findsig(search)) < 0 || (from = tnextvec(s, from)) < 0)) {
	(void)fprintf(stderr, "%s: can't read signal '%s'\n", pname, search);
	exit(2);
    }
    /* Reset 'to' if a duration limit was specified. */
    if (maxl > 0L && (maxl = strtim(argv[maxl])) < 0L)
	maxl = -maxl;
    if (maxl && (to == 0L || to > from + maxl))
	to = from + maxl;

    t = strtim("e"); /* the end of the record */
    if (to == 0L || to > t)
	to = t;

    /* Quit if there will be no output. */
    if (to <= from) {
	wfdbquit();
	fprintf(stderr, "%s: no output\n", pname);
	fprintf(stderr, " Use the -f and -t options to specify time limits\n"
		"(or type '%s -h' for help)\n", pname);
	exit(1);
    }

    /* Generate the names for the output .mat file and the output record. */
    p = record + strlen(record) - 1;	/* *p = final char of record name */
    if (*p == '/')	/* short form name ('rec/' rather than 'rec/rec') */
	sfname = 1;
    while (--p > record)
	if (*p == '/') { p++; break; }  /* omit path components from orec */
    SUALLOC(orec, strlen(p)+2, sizeof(char));
    strncpy(orec, p, strlen(p) - sfname);
    /* If the input record is an EDF file, it will have a '.' in its name.
       The output record will not be an EDF file, so it may not have a '.'
       in its name.  Replace any '.' with '_' here. */
    for (p = orec; *p; p++)
	if (*p == '.') *p = '_';
    *p = 'm';	/* append 'm' to the output record name */
    SUALLOC(matname, strlen(record)+6, sizeof(char));
    sprintf(matname, "%s.mat", orec);

    /* Determine if we can write 8-bit unsigned samples, or if 16 or 32 bits are
       needed per sample. */
    nbytesperelement = 1;
    for (i = 0; i < nosig; i++) {
	if (si[sig[i]].adcres > 0) {
	    if (si[sig[i]].adcres > 16)
		nbytesperelement = 4;
	    else if (si[sig[i]].adcres > 8 && nbytesperelement < 2)
		nbytesperelement = 2;
	}
	else {
	    /* adcres not specified; try to guess from format */
	    if (si[sig[i]].fmt == 24 || si[sig[i]].fmt == 32)
		nbytesperelement = 4;
	    else if (si[sig[i]].fmt != 80 && nbytesperelement < 2)
		nbytesperelement = 2;
	}
    }
    if (nbytesperelement == 1) {
	sub4type = MAT8;
	wfdbtype = 80;
    }
    else if (nbytesperelement == 2) {
	sub4type = MAT16;
	wfdbtype = 16;
    }
    else {
	sub4type = MAT32;
	wfdbtype = 32;
    }

    for (i = 0; i < nosig; i++) {
	so[i] = si[sig[i]];
	so[i].fname = matname;
	so[i].group = 0;
	so[i].spf = 1;
	so[i].fmt = wfdbtype;
	so[i].adczero = 0;
	so[i].baseline -= si[sig[i]].adczero;
	/* handle possibly missing units strings */
	if (so[i].units == NULL) {
	    /* in a .hea file, missing units can be assumed to be millivolts */
	    SSTRCPY(so[i].units, "mV");
	}
	else if (strlen(so[i].units) == 0) {
	    /* this can happen only in an EDF file; in this case, the signal
	       is dimensionless (nd => "no dimension") */
	    SSTRCPY(so[i].units, "nd");
	}
    }


    /* Ensure the signal size does not exceed 2^31 byte limit */
    max_length = 2147483648/nbytesperelement/nosig;
    if ((to - from) > max_length){
	(void)fprintf(stderr, "%s: cannot write mat file -"
		      " data size exceeds 2GB limit\n", pname);
	exit(1);
    }

    nbytesofdata = nbytesperelement*nosig*(to-from);   /* Bytes of actual data */
    lremain = nbytesofdata%8; /* This is the remaining no. bytes that
				 don't fit into integer multiple of
				 8. ie if 18 bytes, lremain=2, from 17
				 to 18. */

    /* nbytesmaster= (8 + 8) + (8 + 8) + (8 + 8) + (8 + nbytesofdata) + padding.
       Must be integer multiple 8. */
    if (lremain==0){
	nbytesmaster = nbytesofdata + 56;
    }
    else{
	nbytesmaster = nbytesofdata + 64 - (lremain);
	for (i = 0; i < nosig; i++)
	    so[i].bsize = 8;
    }

    /* Create an empty .mat file. */
    if (osigfopen(so, nosig) != nosig) {
	wfdbquit();		/* failed to open output, quit */
	exit(1);
    }
    
    /* Fill in the .mat file's prolog and write it. (Elements of prolog[]
       not set explicitly below are always zero.)  */

    /* Start with 128 byte header */
    sprintf(prolog, "MATLAB 5.0");  /* First 116 bits are descriptive text */
    prolog[124] = fieldversion & 0xff; /* Bytes 125-126 indicate version,
					  set to 0x0100 hex = 256.*/
    prolog[125] = (fieldversion >> 8) & 0xff;
    sprintf(prolog + 126, "I");   /* Characters IM to indicate little endian */
    sprintf(prolog + 127, "M");

    /* 8 byte MASTER TAG, followed by the actual data.
       First 4 is data type, next 4 is number of bytes of entire field. */
    prolog[128] = mastertype & 0xff; /* Data type is always 14 for matrix. */
    /* Number of bytes of data element
       = (8 + 8) + (8 + 8) + (8 + 8) + (8 + Nvalues*bytespervalue)
       = 56 + Nvalues*bytespervalue */
    prolog[132] = nbytesmaster & 0xff;
    prolog[133] = (nbytesmaster >> 8) & 0xff;
    prolog[134] = (nbytesmaster >> 16) & 0xff;
    prolog[135] = (nbytesmaster >> 24) & 0xff;

    /* Matrix data has 4 subelements (5 if imag):
       Array flags, dimensions array, array name, real part.
       Each subelement has its own subtag, and subdata. */

    /* Subelement 1: Array flags. */
    prolog[136] = (sub1type & 0xff); /* Sub tag 1: type */
    prolog[140] = 8 & 0xff; /* Sub tag 1: size */
    prolog[144] = sub1class & 0xff; /*Value class, indicating the MATLAB
				      data type. NOT the same as sub4type. */

    /* Subelement 2: Rows and Cols */
    prolog[152] = sub2type & 0xff;  /* Sub tag 2: type */
    prolog[156] = 8 & 0xff; /* sub tag 2: size */
    prolog[160] = nosig & 0xff;  /* Number of signals. */
    prolog[161] = (nosig >> 8) & 0xff;
    prolog[162] = (nosig >> 16) & 0xff;
    prolog[163] = (nosig >> 24) & 0xff;
    prolog[164] = (to - from) & 0xff;   /* Value nrows. */
    prolog[165] = ((to - from) >> 8) & 0xff;
    prolog[166] = ((to - from) >> 16) & 0xff;
    prolog[167] = ((to - from) >> 24) & 0xff;

    /* Subelement 3: Array Name */
    prolog[168] = sub3type & 0xff;
    prolog[172] = 3 & 0xff;
    sprintf(prolog + 176, "val");

    /* Subelement 4: Data itself */
    prolog[184] = sub4type & 0xff;
    prolog[188] = nbytesofdata & 0xff;
    prolog[189] = (nbytesofdata >> 8) & 0xff;
    prolog[190] = (nbytesofdata >> 16) & 0xff;
    prolog[191] = (nbytesofdata >> 24) & 0xff;

    /* Total size of everything before actual data:
       128 byte header
       + 8 byte master tag
       + 56 byte Subelements (48 byte default + 8 byte name)
       = 192. */
    wfdbputprolog((char *)prolog, 192, 0);

    /* Copy the selected data into the .mat file. */
    for (t = from; t < to && stat >= 0; t++) {
	stat = getvec(vi);
	for (i = 0; i < nosig; i++)
	    vo[i] = vi[sig[i]] - si[sig[i]].adczero;
	if (putvec(vo) != nosig)
	    break;
    }

    /* If the input ended prematurely, pad the matrix with invalid samples. */
    if (t != to) {
	fprintf(stderr, "%s (warning): final %ld columns are invalid\n",
		pname, to - t);
	for (i = 0; i < nosig; i++)
	    vo[i] = WFDB_INVALID_SAMPLE;
	while (t++ < to)
	    putvec(vo);
    }

    /* Create the new header file. */
    p = mstimstr(-from);
    if (p && *p == '[')
	setbasetime(p+1);
    newheader(orec);

    /* Copy info from the old record, if any */
    if (p = getinfo(NULL))
	do {
	    (void)putinfo(p);
	} while (p = getinfo((char *)NULL));
    /* Append additional info summarizing what wfdb2mat has done. */
    SUALLOC(p, strlen(record)+80, 1);
    (void)sprintf(p, "Creator: %s", pname);
    (void)putinfo(p);
    (void)sprintf(p, "Source: record %s", record);
    q = mstimstr(0);
    while (*q == ' ') q++;
    if (from != 0 || *q == '[') {
	strcat(p, "  Start: ");
	strcat(p, q);
    }
    (void)putinfo(p);

    /* Determine offset between sample values and the raw byte/word
       values as interpreted by Matlab/Octave. */
    if (wfdbtype == 80)
	offset = 128;
    else
	offset = 0;

    /* Summarize the contents of the .mat file. */
    printf("%s\n", p);
    printf("val has %d row%s (signal%s) and %ld column%s (sample%s/signal)\n",
	   nosig, nosig == 1 ? "" : "s", nosig == 1 ? "" : "s",
	   to-from, to == from+1 ? "" : "s", to == from+1 ? "" : "s");
    printf("Duration: %s\n", timstr(to-from));
    printf("Sampling frequency: %.12g Hz  Sampling interval: %.10g sec\n",
	   freq, 1/freq);
    printf("Row\tSignal\tGain\tBase\tUnits\n");
    for (i = 0; i < nosig; i++)
	printf("%d\t%s\t%.12g\t%d\t%s\n", i+1, so[i].desc, so[i].gain,
	       so[i].baseline + offset, so[i].units);
    printf("\nTo convert from raw units to the physical units shown\n"
	   "above, call the 'rdmat.m' function from the wfdb-matlab\n"
	   "toolbox: https://physionet.org/physiotools/matlab/wfdb-app-matlab/\n");

    SFREE(p);
    wfdbquit();

    exit(0);	/*NOTREACHED*/
}

char *prog_name(s)
char *s;
{
    char *p = s + strlen(s);

#ifdef MSDOS
    while (p >= s && *p != '\\' && *p != ':') {
	if (*p == '.')
	    *p = '\0';		/* strip off extension */
	if ('A' <= *p && *p <= 'Z')
	    *p += 'a' - 'A';	/* convert to lower case */
	p--;
    }
#else
    while (p >= s && *p != '/')
	p--;
#endif
    return (p+1);
}

static char *help_strings[] = {
 "usage: %s -r RECORD [OPTIONS ...]\n",
 "where RECORD is the name of the input record, and OPTIONS may include:",
 " -f TIME     begin at specified time",
 " -h          print this usage summary",
 " -H          read multifrequency signals in high resolution mode",
 " -l INTERVAL truncate output after the specified time interval (hh:mm:ss)",
 " -s SIGNAL   [SIGNAL ...]  convert only the specified signal(s)",
 " -S SIGNAL   search for a valid sample of the specified SIGNAL at or after",
 "		the time specified with -f, and begin converting then",
 " -t TIME     stop at specified time",
"Outputs are written to the current directory as RECORD.mat and RECORDm.hea.",
NULL
};

void help()
{
    int i;

    (void)fprintf(stderr, help_strings[0], pname);
    for (i = 1; help_strings[i] != NULL; i++)
	(void)fprintf(stderr, "%s\n", help_strings[i]);
}
