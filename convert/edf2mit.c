/* file: edf2mit.c		G. Moody	16 October 1996
				Last revised:	   7 May 1999

-------------------------------------------------------------------------------
Convert EDF (European Data Format) file to MIT format header and signal files
Copyright (C) 1999 George B. Moody

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

*/

#include <stdio.h>
#include <wfdb/wfdb.h>

char *pname;

main(argc, argv)
int argc;
char **argv;
{
    static FILE *ifile;
    static char buf[81], record[20];
    int fpb, h, i, j, k, l, nsig, nosig = 0, siglist[WFDB_MAXSIG], spr,
	spb[WFDB_MAXSIG], tspb = 0, tspf = 0, vflag = 0;
    char **vi, **vin;
    WFDB_Sample *vo, *vout;
    int day, month, year, hour, minute, second;
    long adcrange, sigdmax[WFDB_MAXSIG], sigdmin[WFDB_MAXSIG];
    double sigpmax[WFDB_MAXSIG], sigpmin[WFDB_MAXSIG], sampfreq[WFDB_MAXSIG],
        sps;
    static WFDB_Siginfo si[WFDB_MAXSIG], so[WFDB_MAXSIG];
    void help();

    pname = argv[0];
    for (i = 1; i < argc; i++) {
	if (*argv[i] == '-') switch (argv[i][1]) {
	  case 'h':	/* show usage and quit */
	    help();
	    exit(0);
	  case 'i':	/* file name follows */
	    if (++i < argc) {
		if (strcmp(argv[i], "-"))
		    ifile = stdin;
		else {
		    ifile = fopen(argv[i], "rb");
		    if (ifile == NULL) {
			fprintf(stderr, "%s: can't read %s\n", pname, argv[i]);
			exit(1);
		    }
		}
	    }
	    break;
	  case 'r':	/* record name follows */
	    if (++i < argc)
		strncpy(record, argv[i], 19);
	    else {
		fprintf(stderr, "%s: record name must follow -r\n", pname);
		exit(1);
	    }
	    break;
	  case 's':	/* signal list follows */
	    while (++i < argc && argv[i][0] != '-')
		siglist[nosig++] = atoi(argv[i]);
	    --i;
	    break;
	  case 'v':	/* select verbose mode */
	    vflag = 1;
	    break;
	}
    }

    if (ifile == NULL) {
	help();
	exit(1);
    }
    fread(buf, 1, 8, ifile);
    buf[8] = ' ';
    for (j = 8; j >= 0 && buf[j] == ' '; j--)
	buf[j] = '\0';
    if (vflag) printf("EDF version number: '%s'\n", buf);

    fread(buf, 1, 80, ifile);
    buf[80] = ' ';
    for (j = 80; j >= 0 && buf[j] == ' '; j--)
	buf[j] = '\0';
    if (vflag) printf("Patient ID: '%s'\n", buf);

    fread(buf, 1, 80, ifile);
    buf[80] = ' ';
    for (j = 80; j >= 0 && buf[j] == ' '; j--)
	buf[j] = '\0';
    if (vflag) printf("Recording ID: '%s'\n", buf);
    if (record[0] == 0) {
	strncpy(record, buf, 19);
	for (j = 0; j < 20; j++)
	    if (record[j] == ' ') { record[j] = '\0'; break; }
    }

    fread(buf, 1, 8, ifile);
    buf[8] = ' ';
    for (j = 8; j >= 0 && buf[j] == ' '; j--)
	buf[j] = '\0';
    if (vflag) printf("Recording date: '%s'\n", buf);
    sscanf(buf, "%d%*c%d%*c%d", &day, &month, &year);
    if (year < 1970) year += 1900;
    if (year < 1970) year += 100;	/* should work for a while */

    fread(buf, 1, 8, ifile);
    buf[8] = ' ';
    for (j = 8; j >= 0 && buf[j] == ' '; j--)
	buf[j] = '\0';
    if (vflag) printf("Recording time: '%s'\n", buf);
    sscanf(buf, "%d%*c%d%*c%d", &hour, &minute, &second);

    fread(buf, 1, 8, ifile);
    buf[8] = ' ';
    for (j = 8; j >= 0 && buf[j] == ' '; j--)
	buf[j] = '\0';
    if (vflag) printf("Number of bytes in header record: '%s'\n", buf);

    fread(buf, 1, 44, ifile);
    buf[44] = ' ';
    for (j = 44; j >= 0 && buf[j] == ' '; j--)
	buf[j] = '\0';
    if (j > 0)
    if (vflag) printf("Free space: '%s'\n", buf);

    fread(buf, 1, 8, ifile);
    buf[8] = ' ';
    for (j = 8; j >= 0 && buf[j] == ' '; j--)
	buf[j] = '\0';
    if (vflag) printf("Number of data records: '%s'\n", buf);

    fread(buf, 1, 8, ifile);
    buf[8] = ' ';
    for (j = 8; j >= 0 && buf[j] == ' '; j--)
	buf[j] = '\0';
    if (vflag) printf("Duration of each data record in seconds: '%s'\n", buf);
    sscanf(buf, "%d", &spr);
    if (spr < 1) spr = 1;

    fread(buf, 1, 4, ifile);
    buf[4] = ' ';
    for (j = 4; j >= 0 && buf[j] == ' '; j--)
	buf[j] = '\0';
    if (vflag) printf("Number of signals: '%s'\n", buf);
    sscanf(buf, "%d", &nsig);

    if (nsig < 1) exit(1);
    if (nsig > WFDB_MAXSIG) {
	fprintf(stderr, "Too many signals!\n");
	exit(2);
    }

    for (i = 0; i < nsig; i++) {
	fread(buf, 1, 16, ifile);
	buf[16] = ' ';
	for (j = 15; j >= 0 && buf[j] == ' '; j--)
	    buf[j] = '\0';
	if (vflag) printf("Signal %d label: '%s'\n", i, buf);
	si[i].desc = (char *)malloc(strlen(buf)+1);
	if (si[i].desc) strcpy(si[i].desc, buf);
    }

    for (i = 0; i < nsig; i++) {
	fread(buf, 1, 80, ifile);
	buf[80] = ' ';
	for (j = 80; j >= 0 && buf[j] == ' '; j--)
	    buf[j] = '\0';
	if (vflag && j > 0)
	    printf("Signal %d transducer type: '%s'\n", i, buf);
    }
    
    for (i = 0; i < nsig; i++) {
	fread(buf, 1, 8, ifile);
	buf[8] = ' ';
	for (j = 8; j >= 0 && buf[j] == ' '; j--)
	    buf[j] = '\0';
	if (vflag) printf("Signal %d physical dimension: '%s'\n", i, buf);
	si[i].units = (char *)malloc(strlen(buf)+1);
	if (si[i].units) strcpy(si[i].units, buf);
    }

    for (i = 0; i < nsig; i++) {
	fread(buf, 1, 8, ifile);
	buf[8] = ' ';
	for (j = 8; j >= 0 && buf[j] == ' '; j--)
	    buf[j] = '\0';
	if (vflag) printf("Signal %d physical minimum: '%s'\n", i, buf);
	sscanf(buf, "%lf", &sigpmin[i]);
    }

    for (i = 0; i < nsig; i++) {
	fread(buf, 1, 8, ifile);
	buf[8] = ' ';
	for (j = 8; j >= 0 && buf[j] == ' '; j--)
	    buf[j] = '\0';
	if (vflag) printf("Signal %d physical maximum: '%s'\n", i, buf);
	sscanf(buf, "%lf", &sigpmax[i]);
    }

    for (i = 0; i < nsig; i++) {
	fread(buf, 1, 8, ifile);
	buf[8] = ' ';
	for (j = 8; j >= 0 && buf[j] == ' '; j--)
	    buf[j] = '\0';
	if (vflag) printf("Signal %d digital minimum: '%s'\n", i, buf);
	sscanf(buf, "%d", &sigdmin[i]);
    }

    for (i = 0; i < nsig; i++) {
	fread(buf, 1, 8, ifile);
	buf[8] = ' ';
	for (j = 8; j >= 0 && buf[j] == ' '; j--)
	    buf[j] = '\0';
	if (vflag) printf("Signal %d digital maximum: '%s'\n", i, buf);
	sscanf(buf, "%d", &sigdmax[i]);
    }

    for (i = 0; i < nsig; i++) {
	if (sigpmax[i] == sigpmin[i]) {
	    fprintf(stderr, "gain for signal %d is undefined\n", i);
	    continue;
	}
	si[i].adczero = (sigdmax[i]+1 + sigdmin[i])/2;
	adcrange = sigdmax[i]+1 - sigdmin[i];
	si[i].gain = adcrange/(sigpmax[i] - sigpmin[i]);
	for (j = 0; adcrange > 1; j++)
	    adcrange /= 2;
	si[i].adcres = j;
	si[i].baseline = sigdmax[i] - sigpmax[i]*si[i].gain + 1;
    }

    for (i = 0; i < nsig; i++) {
	fread(buf, 1, 80, ifile);
	buf[80] = ' ';
	for (j = 80; j >= 0 && buf[j] == ' '; j--)
	    buf[j] = '\0';
	if (vflag && j > 0)
	    printf("Signal %d filtering information: '%s'\n", i, buf);
    }

    sps = 0.0;
    for (i = 0; i < nsig; i++) {
	fread(buf, 1, 8, ifile);
	buf[8] = ' ';
	for (j = 8; j >= 0 && buf[j] == ' '; j--)
	    buf[j] = '\0';
	if (vflag)
	    printf("Signal %d number of samples per record: '%s'\n", i, buf);
	sscanf(buf, "%lf", &sampfreq[i]);
	sscanf(buf, "%d", &spb[i]);
	tspb += spb[i];
	sampfreq[i] /= spr;
    }

    for (i = 0; i < nsig; i++) {
	fread(buf, 1, 32, ifile);
	buf[32] = ' ';
	for (j = 32; j >= 0 && buf[j] == ' '; j--)
	    buf[j] = '\0';
	if (vflag && j > 0)
	    printf("Signal %d free space: '%s'\n", i, buf);
    }

    if (nosig == 0)	/* initialize signal list if necessary */
	for ( ; nosig < nsig; nosig++)
	    siglist[nosig] = nosig;

    /* Determine the base sampling frequency (the lowest sampling frequency for
       any signal in the signal list) */
    sps = 0;
    for (i = 0; i < nosig; i++)
	if (sampfreq[siglist[i]] > sps) sps = sampfreq[siglist[i]];
    for (i = 0; i < nosig; i++)
	if (sampfreq[siglist[i]] <= sps) {
	    sps = sampfreq[siglist[i]];
	    fpb = spb[siglist[i]];
	}
    setsampfreq(sps);

    sprintf(buf, "%02d:%02d:%02d %02d/%02d/%04d",
	    hour, minute, second, day, month, year);
    setbasetime(buf);

    sprintf(buf, "%s.dat", record);
    for (i = 0; i < nosig; i++) {
	si[siglist[i]].fname = buf;
	si[siglist[i]].fmt = 16;
	tspf += si[siglist[i]].spf = sampfreq[siglist[i]]/sps;
	so[i] = si[siglist[i]];
    }

    vin = (char **)malloc(nsig * sizeof(char *));
    vi = (char **)malloc(nsig * sizeof(char *));
    for (i = 0; i < nsig; i++) {
	vin[i] = (char *)malloc(spb[i] * 2);
    }
    vout = (WFDB_Sample *)malloc(tspf * sizeof(WFDB_Sample));

    osigfopen(so, nosig);
    while (1) {
	for (i = 0; i < nsig; i++)
	    j = fread(vin[i], 2, spb[i], ifile);
	if (j == 0) break;
	for (j = 0; j < nosig; j++)
	    vi[siglist[j]] = vin[siglist[j]];
	for (i = 0; i < fpb; i++) {
	    vo = vout;
	    for (j = 0; j < nosig; j++)
		for (k = 0; k < so[j].spf; k++) {
		    h = *vi[siglist[j]]++;	/* high byte first */
		    l = *vi[siglist[j]]++;	/* then low byte */
		    /* If a short int is not 16 bits, it may be necessary to
		       modify the next line for proper sign extension. */
		    *vo++ = ((int)((short)((h << 8) | (l &0xff))));
		}
	    putvec(vout);
	}
    }
    if (ifile != stdin) fclose(ifile);

    newheader(record);
    wfdbquit();
}

static char *help_strings[] = {
 "usage: %s [OPTIONS ...]\n",
 "where OPTIONS may include:",
 " -h          print this usage summary",
 " -i EDFILE   read the specified European Data Format file",
 " -r RECORD   create the specified RECORD (default: use patient id)",
 " -s SIGNAL [SIGNAL ...]  copy only the specified signal(s) (use signal",
 "              numbers, beginning with zero;  default: copy all signals)",
 " -v          print debugging output",
 "This program reads an EDF file and creates MIT-format signal and header",
 "files containing the same data.",
NULL
};

void help()
{
    int i;

    (void)fprintf(stderr, help_strings[0], pname);
    for (i = 1; help_strings[i] != NULL; i++)
	(void)fprintf(stderr, "%s\n", help_strings[i]);
}
