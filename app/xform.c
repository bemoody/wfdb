/* file: xform.c	G. Moody       8 December 1983
			Last revised:  2 October 1999

-------------------------------------------------------------------------------
xform: Sampling frequency, amplitude, and format conversion for WFDB records
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
#ifdef __STDC__
# include <stdlib.h>
#else
extern void exit();
# ifndef NOMALLOC_H
# include <malloc.h>
# else
extern char *malloc();
# endif
#endif
#ifndef BSD
# include <string.h>
#else
# include <strings.h>
#endif

#include <wfdb/wfdb.h>

char *pname, *prog_name();
int gcd();
void help();

main(argc, argv)
int argc;
char *argv[];
{
    char *irec = NULL, *orec = NULL, *nrec = NULL, *startp = "0:0";
    int i;
    int fflag = 0, gflag = 0, Hflag = 0, ifreq, j, m, msiglist[WFDB_MAXSIG],
	Mflag = 0, mn, n, nann = 0, nisig, nminutes = 0, nosig = -1, ofreq = 0,
        reopen = 0, siglist[WFDB_MAXSIG], spf = 1, use_irec_desc = 1;
    static char btstring[30];
    static double gain[WFDB_MAXSIG];
    static int deltav[WFDB_MAXSIG], v[WFDB_MAXSIG], vin[WFDB_MAXSIG*WFDB_MAXSPF],
        vmax[WFDB_MAXSIG], vmin[WFDB_MAXSIG], vv[WFDB_MAXSIG],
	vout[WFDB_MAXSIG*WFDB_MAXSPF];
    static long it, ot, from, to, nsamp = -1L, spm, nsm = 0L;
    static WFDB_Siginfo dfin[WFDB_MAXSIG], dfout[WFDB_MAXSIG];
    static WFDB_Anninfo ai[WFDB_MAXANN];
    static WFDB_Annotation annot;

    pname = prog_name(argv[0]);

    /* Interpret the command-line arguments. */
    for (i = 1; i < argc; i++) {
	if (*argv[i] == '-') switch (*(argv[i]+1)) {
	  case 'a':	/* annotator(s) */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: annotator(s) must follow -a\n",
			      pname);
		exit(1);
	    }
	    /* Accept the next argument unconditionally as an annotator name;
	       accept additional arguments until we find one beginning with
	       `-', or the end of the argument list. */
	    do {
		if (nann >= WFDB_MAXANN) {
		    (void)fprintf(stderr,
	           "%s: no more than %d annotators may be processed at once\n",
				  pname, WFDB_MAXANN);
		    exit(1);
		}
		ai[nann].name = argv[i++]; ai[nann++].stat = WFDB_READ;
	    } while (i < argc && *argv[i] != '-');
	    i--;
	    break;
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
	  case 'H':	/* open the input record in `high resolution' mode */
	    Hflag = 1;
	    break;
	  case 'i':	/* input record name follows */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: input record name must follow -i\n",
			pname);
		exit(1);
	    }
	    irec = argv[i];
	    break;
	  case 'M':	/* multifrequency mode (no frequency changes) */
	    Mflag = 1;
	    break;
	  case 'n':	/* new record name follows */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: new record name must follow -n\n",
			pname);
		exit(1);
	    }
	    nrec = argv[i];
	    break;
	  case 'N':	/* new record name follows;  use signal descriptions
			   from output record header file in new header file */
	    use_irec_desc = 0;
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: new record name must follow -N\n",
			pname);
		exit(1);
	    }
	    nrec = argv[i];
	    break;
	  case 'o':	/* output record name follows */
	    if (++i >= argc) {
		(void)fprintf(stderr,"%s: output record name must follow -o\n",
			pname);
		exit(1);
	    }
	    orec = argv[i];
	    break;
	  case 's':	/* signal list follows */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: signal list must follow -s\n",
			pname);
		exit(1);
	    }
	    nosig = 0;
	    while (i < argc && *argv[i] != '-') {
		if (nosig == WFDB_MAXSIG) {
		    (void)fprintf(stderr, "%s: too many output signals\n",
				  pname);
		    exit(1);
		}
		siglist[nosig++] = atoi(argv[i++]);
	    }
	    i--;
	    break;
	  case 't':	/* end time */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: time must follow -t\n", pname);
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

    /* Check that an input record was specified. */
    if (irec == NULL) {
	help();
	exit(1);
    }

    /* Get input signal information and sampling frequency to use as defaults
       for output. */
    nisig = isigopen(irec, dfin, WFDB_MAXSIG);
    if (Hflag) {
	setgvmode(WFDB_HIGHRES);
	spf = getspf();
    }
    ifreq = strtim("1") * spf;
    (void)setsampfreq(0.);

    /* If no output record was specified, get the specifications
       interactively. */
    if (orec == NULL) {
	char answer[32], directory[32];
	static char filename[WFDB_MAXSIG][32], description[WFDB_MAXSIG][32],
	    record[8], units[WFDB_MAXSIG][22];
	static int formats[WFDB_NFMTS] = WFDB_FMT_LIST;	/* see <wfdb/wfdb.h> */
	FILE *ttyin = NULL;

#ifndef MSDOS
	ttyin = fopen("/dev/tty", "r");
#endif
#ifdef MSDOS
	ttyin = fopen("CON", "rt");
#endif
	if (ttyin == NULL) ttyin = stdin;
	if (nrec == NULL) {
	    do {
		(void)fprintf(stderr,
		 "Choose a name for the output record (up to 6 characters): ");
		(void)fgets(record, 8, ttyin); record[strlen(record)-1] = '\0';
	    } while (record[0] == '\0' || newheader(record) < 0);
	    nrec = record;
	}
	if (nosig == -1) {
	    do {
		nosig = nisig;
		(void)fprintf(stderr,
			      "Number of signals to be written (1-%d) [%d]: ",
		       WFDB_MAXSIG, nosig);
		(void)fgets(answer, 32, ttyin);
		(void)sscanf(answer, "%d", &nosig);
		if (nosig == 0) {
		    (void)fprintf(stderr,
			"No signals will be written.  Are you sure? [n]: ");
		    (void)fgets(answer, 32, ttyin);
		    if (*answer != 'y' && *answer != 'Y')
			nosig = -1;
		}
	    } while (nosig < 0 || nosig > WFDB_MAXSIG);
	    for (i = 0; i < nosig; i++)
		siglist[i] = i;
	}
	if (Mflag) {
	    ofreq = ifreq;
	    (void)fprintf(stderr,
		  "Sampling frequency (%d frames/sec) will be unchanged\n",
			  ofreq);
	}
	else do {
	    ofreq = ifreq;
	    (void)fprintf(stderr,
		  "Output sampling frequency (Hz per signal, > 0) [%d]: ",
			  ofreq);
	    (void)fgets(answer, 32, ttyin); (void)sscanf(answer, "%d", &ofreq);
	} while (ofreq < 0);
	if (ofreq == 0) ofreq = WFDB_DEFFREQ;
	if (nosig > 0) {
	    (void)fprintf(stderr,
		"Specify the name of the directory in which the output\n");
	    (void)fprintf(stderr,
		" signals should be written, or press <return> to write\n");
	    (void)fprintf(stderr, " them in the current directory: ");
	    (void)fgets(directory, 31, ttyin);
	    if (*directory == '\n') *directory = '\0';
	    else directory[strlen(directory)-1] = '/';
	    (void)fprintf(stderr,"Any of these output formats may be used:\n");
	    (void)fprintf(stderr, "    8   8-bit first differences\n");
	    (void)fprintf(stderr,
		"   16   16-bit two's complement amplitudes (LSB first)\n");
	    (void)fprintf(stderr,
		"   61   16-bit two's complement amplitudes (MSB first)\n");
	    (void)fprintf(stderr, "   80   8-bit offset binary amplitudes\n");
	    (void)fprintf(stderr, "  160   16-bit offset binary amplitudes\n");
	    (void)fprintf(stderr,
		        "  212   2 12-bit amplitudes bit-packed in 3 bytes\n");
	    (void)fprintf(stderr,
  "  310   3 10-bit amplitudes bit-packed in 15 LS bits of each of 2 words\n");
	    (void)fprintf(stderr,
	  "  311   3 10-bit amplitudes bit-packed in 30 LS bits of 4 bytes\n");
	    do {
		dfout[0].fmt = dfin[0].fmt;
		(void)fprintf(stderr,
		 "Choose an output format (8/16/61/80/160/212/310/311) [%d]: ",
			      dfout[0].fmt);
		(void)fgets(answer, 32, ttyin);
		(void)sscanf(answer, "%d", &dfout[0].fmt);
		for (i = 0; i < WFDB_NFMTS; i++)
		    if (dfout[0].fmt == formats[i]) break;
	    } while (i >= WFDB_NFMTS);
	    if (nosig > 1) {
		(void)fprintf(stderr, "Save all signals in one file? [y]: ");
		(void)fgets(answer, 32, ttyin);
	    }
	    if (nosig <= 1 || (answer[0] != 'n' && answer[0] != 'N')) {
		(void)sprintf(filename[0], "%s%s.dat", directory, nrec);
		for (i = 0; i < nosig; i++) {
		    dfout[i].fname = filename[0];
		    dfout[i].group = 0;
		}
	    }
	    else {
		for (i = 0; i < nosig; i++) {
		    (void)sprintf(filename[i], "%s%s.d%d", directory, nrec, i);
		    dfout[i].fname = filename[i];
		    dfout[i].group = i;
		}
	    }
	}
	for (i = 0; i < nosig; i++) {
            dfout[i].fmt = dfout[0].fmt; dfout[i].bsize = 0;
	    dfout[i].gain = dfin[siglist[i]].gain;
	    dfout[i].adcres = dfin[siglist[i]].adcres;
	    dfout[i].adczero = dfin[siglist[i]].adczero;
	    dfout[i].units = dfin[siglist[i]].units;
	    if (Mflag) dfout[i].spf = dfin[siglist[i]].spf;
	    if (!use_irec_desc) {
		char *p;

		(void)fprintf(stderr,
			"Signal %d description (up to 30 characters): ", i);
		(void)fgets(description[i], 32, ttyin);
		description[i][strlen(description[i])-1] = '\0';
		dfout[i].desc = description[i];
		(void)fprintf(stderr,
			      "Signal %d units (up to 20 characters): ", i);
		(void)fgets(units[i], 22, ttyin);
		for (p = units[i]; *p; p++) {
		    if (*p == ' ' || *p == '\t') *p = '_';
		    else if (*p == '\n') { *p = '\0'; break; }
		}
		dfout[i].units = *units[i] ? units[i] : NULL;
	    }
	    do {
		(void)fprintf(stderr, " Signal %d gain (adu/%s) [%g]: ",
			      i, dfout[i].units ? dfout[i].units : "mV",
			      dfout[i].gain);
		(void)fgets(answer, 32, ttyin);
		(void)sscanf(answer, "%lf", &dfout[i].gain);
	    } while (dfout[i].gain < 0.);
	    do {
		if (i > 0) dfout[i].adcres = dfout[i-1].adcres;
		switch (dfout[i].fmt) {
		  case 80:
		    dfout[i].adcres = 8;
		    break;
		  case 212:
		    if (dfout[i].adcres < 8 || dfout[i].adcres > 12)
			dfout[i].adcres = 12;
		    break;
		  case 310:
		    if (dfout[i].adcres < 8 || dfout[i].adcres > 10)
			dfout[i].adcres = 10;
		    break;
		  default:
		    if (dfout[i].adcres < 8 || dfout[i].adcres > 16)
			dfout[i].adcres = WFDB_DEFRES;
		    break;
		}
		(void)fprintf(stderr,
			" Signal %d ADC resolution in bits (8-16) [%d]: ", i,
			      dfout[i].adcres);
		(void)fgets(answer, 32, ttyin);
		(void)sscanf(answer, "%d", &dfout[i].adcres);
	    } while (dfout[i].adcres < 8 || dfout[i].adcres > 16);
	    (void)fprintf(stderr, " Signal %d ADC zero level (adu) [%d]: ",
			  i, dfout[i].adczero);
	    (void)fgets(answer, 32, ttyin);
	    (void)sscanf(answer, "%d", &dfout[i].adczero);
	}
    }

    /* Check that the output record can be created. */
    if (nrec && newheader(nrec) < 0) exit(2);

    /* Determine the output sampling frequency. */
    if (orec) ofreq = sampfreq(orec);
    (void)setsampfreq(ifreq/spf);

    if (ifreq != ofreq) {
	if (Mflag != 0) {
	    fprintf(stderr,
      "%s: -M option may not be used if sampling frequency is to be changed\n",
		    pname);
	    wfdbquit();
	    exit(1);
	}
    }

    /* Check the starting and ending times. */
    if (from) {
	startp = argv[(int)from];
	from = strtim(startp);
	if (from < 0L) from = -from;
	strcpy(btstring, timstr(-from));
    }
    if (to) {
	to = strtim(argv[(int)to]);
	if (to < 0L) to = -to;
	if (to > 0L && (nsamp = to - from) <= 0L) {
	    (void)fprintf(stderr,
			  "%s: 'to' time must be later than 'from' time\n",
			  pname);
	    exit(1);
	}
    }

    spm = strtim("1:0");

    /* Process the annotation file(s), if any. */
    if (nann > 0) {
	char *p0, *p1;
	int cc;
	long tt;

	/* Check that the input annotation files are readable. */
	if (annopen(irec, ai, (unsigned)nann) < 0 || iannsettime(from) < 0)
	    exit(2);

	/* Associate the output annotation files with the new record, if a
	   new record name was specified. */
	if (nrec) p0 = nrec;
	/* Otherwise, associate them with the existing output record. */
	else p0 = orec;

	/* Check that the output annotation files are writeable, without
	   closing the input annotation files. */
	if ((p1 = (char *)malloc((unsigned)strlen(p0)+2)) == NULL) {
	    (void)fprintf(stderr, "%s: insufficient memory\n", pname);
	    exit(2);
	}
	(void)sprintf(p1, "+%s", p0);
	for (i = 0; i < nann; i++)
	    ai[i].stat = WFDB_WRITE;
	if (annopen(p1, ai, (unsigned)nann) < 0) exit(2);

	/* Write the annotation files. */
	for (i = 0; i < nann; i++) {
	    tt = 0L;
	    cc = -129;
	    while (getann((unsigned)i, &annot) == 0 &&
		   (to == 0L || annot.time <= to)) {
		annot.time = (annot.time - from) * ((double)ofreq*spf) / ifreq;
		/* Make sure that the corrected annotation time is positive
		   and that it is later than the previous annotation time
		   (exception: if it matches the previous annotation time,
		   but the chan field is greater than that of the previous
		   annotation, no time adjustment is performed). */
		if (annot.time < tt || (annot.time == tt && annot.chan <= cc))
		    annot.time = ++tt;
		else tt = annot.time;
		cc = annot.chan;
		(void)putann((unsigned)i, &annot);
	    }
	}

	/* Close the annotation files. */
	for (i = nann-1; i >= 0; i--) {
	    iannclose(i);
	    oannclose(i);
	}
    }

    /* Quit if no signals are to be written. */
    if (nosig == 0) exit(0);

    /* Check that the input signals are readable. */
    fprintf(stderr, "Checking input signals ...");
    if (isigsettime(from) < 0)
	exit(2);
    fprintf(stderr, " done\n");

    /* If no signal list was specified, generate one. */
    if (nosig == -1)
	for (nosig = 0; nosig < nisig; nosig++)
	    siglist[nosig] = nosig;

    /* Otherwise, check that the signal numbers are legal. */
    else {
	for (i = 0; i < nosig && nosig > 0; i++)
	    if (siglist[i] < 0 || siglist[i] >= nisig) {
		(void)fprintf(stderr,
		    "%s: warning: signal %d can't be read from record %s\n",
			pname, siglist[i], irec);
		/* Delete illegal signal numbers from the list. */
		for (j = i; j+1 < nosig; j++)
		    siglist[j] = siglist[j+1];
		nosig--;
		i--;
	    }
	if (nosig == 0) {
	    (void)fprintf(stderr, "%s: no output signals written\n", pname);
	    exit(2);
	}
    }

    /* If an output record was specified using `-o', check that the output
       signals are writable. */
    if (orec) {
	/* Temporarily suppress WFDB library error messages. */
	wfdbquiet();

	n = nosig;
	while ((i = osigopen(orec, dfout, (unsigned)n)) != n) {
	    if (i == -1) {
		(void)fprintf(stderr,
			      "%s: can't read header file for record %s\n",
			pname, orec);
		exit(2);
	    }
	    else if (i == -2) {
		(void)fprintf(stderr,
			"%s: incorrect header file format for record %s\n",
			pname, orec);
		exit(2);
	    }
	    else if (--n <= 0) {
		(void)fprintf(stderr,
			"%s: can't open any output signals for record %s\n",
			pname, orec);
		exit(2);
	    }
	}

	/* If some but not all outputs can be written, print a warning and
	   reduce the number of outputs as necessary. */
	if (n < nosig) {
	    (void)fprintf(stderr,
		    "%s: warning: writing only the first %d of %d signals\n",
		    pname, n, nosig);
	    nosig = n;
	    for (nisig = i = 0; i < nosig; i++)
		if (nisig <= siglist[i]) nisig = siglist[i]+1;
	}

	/* Re-enable WFDB library error messages. */
	wfdbverbose();
    }

    /* If the `-n' option was specified, copy the signal descriptions from the
       input record header file into the siginfo structures for the output
       record (for eventual storage in the new output header file.)  To make
       the changes effective, the output signals must be (re)opened using
       osigfopen. */
    if (nrec && use_irec_desc) {
	char *p;

	for (i = 0; i < nosig; i++) {
	    dfout[i].desc = dfin[siglist[i]].desc;
	    /* If the filenames were obtained from osigopen, they must be
	       copied (since osigfopen returns the storage allocated for them
	       by osigopen to the heap). */
	    if ((p = (char *)malloc((unsigned)strlen(dfout[i].fname)+1)) ==
		(char *)NULL) {
		(void)fprintf(stderr, "%s: out of memory\n", pname);
		exit(2);
	    }
	    (void)strcpy(p, dfout[i].fname);
	    dfout[i].fname = p;
	}
	reopen = 1;
    }

    /* Determine relative gains and offsets of the signals.  If any output
       gain must be adjusted, the output signals must be reopened (to ensure
       that the correct gains are written to the output header). */
    for (i = 0; i < nosig; i++) {
	j = siglist[i];
	/* A signal may be rescaled by xform depending on the gains and ADC
	   resolutions specified.  The rules for doing so are as follows:
	   1.  If the output gain has not been specified, the signal is scaled
	       only if the input and output ADC resolutions have been specified
	       and differ.  In this case, the scale factor (gain[]) is
	       determined so that the appropriate number of bits will be
	       dropped from or added to each signal.
	   2.  If the output gain has been specified, the signal is scaled by
	       the ratio of the output and input gains.  If the input gain is
	       not specified explicitly, the value WFDB_DEFGAIN is assumed.
	   These rules differ from those used by earlier versions of xform,
	   although the default behavior remains the same in most cases. */

	if (dfout[i].gain == 0.) {	/* output gain not specified */
	    if (dfin[j].adcres == 0) {	/* input ADC resolution not specified;
					   assume a suitable default */
		switch (dfin[j].fmt) {
		  case 80:
		    dfin[j].adcres = 8;
		    break;
		  case 212:
		    dfin[j].adcres = 12;
		    break;
		  case 310:
		    dfin[j].adcres = 10;
		    break;
		  default:
		    dfin[j].adcres = WFDB_DEFRES;
		    break;
		}
	    }
	    gain[i] = (double)(1L << dfout[i].adcres) /
		      (double)(1L << dfin[j].adcres);
	}
	else if (dfin[j].gain == 0.)	/* output gain specified, but input
					   gain not specified */
	    gain[i] = dfout[i].gain/WFDB_DEFGAIN;
	else				/* input and output gains specified */
	    gain[i] = dfout[i].gain/dfin[j].gain;

	if (gain[i] != 1.) gflag = 1;
        deltav[i] = dfout[i].adczero - gain[i] * dfin[j].adczero;
	if (dfout[i].gain != dfin[j].gain * gain[i]) {
	    dfout[i].gain = dfin[j].gain * gain[i];
	    reopen = 1;
	}
	/* Note that dfout[i].gain is 0 (indicating that the signal is
	   uncalibrated) if dfin[j].gain is 0.  This is a *feature*, not
	   a bug:  we don't want to create output signals that appear to
	   have been calibrated if the inputs weren't calibrated. */
    }

    /* Reopen output signals if necessary. */
    if (reopen && osigfopen(dfout, (unsigned)nosig) != nosig)
	exit(2);

    /* Determine the legal range of sample values for each output signal. */
    for (i = 0; i < nosig; i++) {
	vmax[i] = dfout[i].adczero + (1 << (dfout[i].adcres-1)) - 1;
	vmin[i] = dfout[i].adczero - (1 << (dfout[i].adcres-1));
    }

    /* If resampling is required, initialize the interpolation/decimation
       parameters. */
    if (ifreq != ofreq) {
	fflag = 1;
	i = gcd(ifreq, ofreq);
	m = ifreq/i;
	n = ofreq/i;
	mn = m*n;
	(void)getvec(vin);
	for (i = 0; i < nosig; i++) 
	    vv[i] = vin[siglist[i]]*gain[i] + deltav[i];	
    }

    /* If in multifrequency mode, set msiglist offsets. */
    if (Mflag) {
	int j, k;

	for (i = 0; i < nosig; i++) {
	    for (j = k = 0; j < siglist[i]; j++)
		k += dfin[j].spf;
	    msiglist[i] = k;
	}
    }

    /* Process the signals. */
    if (fflag == 0 && gflag == 0) {	/* no frequency or gain changes */
      if (Mflag == 0) {			/* standard mode */
	while (getvec(vin) >= nisig && (nsamp == -1L || nsamp-- > 0L)) {
	    if (++nsm >= spm) {
		nsm = 0L;
		(void)fprintf(stderr, ".");
		(void)fflush(stderr);
		if (++nminutes >= 60) {
		    nminutes = 0;
		    (void)fprintf(stderr, "\n");
		}
	    }
	    for (i = 0; i < nosig; i++) {
		vout[i] = vin[siglist[i]] + deltav[i];
		if (vout[i] > vmax[i]) {
		    (void)fprintf(stderr, "v[%d] = %d (out of range)\n",
				  i, vout[i]);
		    vmax[i] = vout[i];
		}
		else if (vout[i] < vmin[i]) {
		    (void)fprintf(stderr, "v[%d] = %d (out of range)\n",
				  i, vout[i]);
		    vmin[i] = vout[i];
		}
	    }
	    if (putvec(vout) < 0) break;
	}
      }
      else {				/* multifrequency mode */
	while (getframe(vin) >= nisig && (nsamp == -1L || nsamp-- > 0L)) {
	    int j, k;

	    if (++nsm >= spm) {
		nsm = 0L;
		(void)fprintf(stderr, ".");
		(void)fflush(stderr);
		if (++nminutes >= 60) {
		    nminutes = 0;
		    (void)fprintf(stderr, "\n");
		}
	    }
	    for (i = j = 0; i < nosig; i++) {
	      for (k = 0; k < dfout[i].spf; j++, k++) {
		vout[j] = vin[msiglist[i] + k] + deltav[i];
		if (vout[j] > vmax[i]) {
		    (void)fprintf(stderr, "v[%d] = %d (out of range)\n",
				  i, vout[j]);
		    vmax[i] = vout[j];
		}
		else if (vout[j] < vmin[i]) {
		    (void)fprintf(stderr, "v[%d] = %d (out of range)\n",
				  i, vout[j]);
		    vmin[i] = vout[j];
		}
	      }
	    }
	    if (putvec(vout) < 0) break;
	}
      }
    }
    else if (fflag == 0 && gflag != 0) {	/* change gain only */
      if (Mflag == 0) {			/* standard mode */
	while (getvec(vin) >= nisig && (nsamp == -1L || nsamp-- > 0L)) {
	    if (++nsm >= spm) {
		nsm = 0L;
		(void)fprintf(stderr, ".");
		(void)fflush(stderr);
		if (++nminutes >= 60) {
		    nminutes = 0;
		    (void)fprintf(stderr, "\n");
		}
	    }
	    for (i = 0; i < nosig; i++) {
		vout[i] = vin[siglist[i]]*gain[i] + deltav[i];
		if (vout[i] > vmax[i]) {
		    (void)fprintf(stderr, "v[%d] = %d (out of range)\n",
				  i, vout[i]);
		    vmax[i] = vout[i];
		}
		else if (vout[i] < vmin[i]) {
		    (void)fprintf(stderr, "v[%d] = %d (out of range)\n",
				  i, vout[i]);
		    vmin[i] = vout[i];
		}
	    }
	    if (putvec(vout) < 0) break;
	}
      }
      else {				/* multifrequency mode */
	while (getframe(vin) >= nisig && (nsamp == -1L || nsamp-- > 0L)) {
	    int j, k;

	    if (++nsm >= spm) {
		nsm = 0L;
		(void)fprintf(stderr, ".");
		(void)fflush(stderr);
		if (++nminutes >= 60) {
		    nminutes = 0;
		    (void)fprintf(stderr, "\n");
		}
	    }
	    for (i = j = 0; i < nosig; i++) {
	      for (k = 0; k < dfout[i].spf; j++, k++) {
		vout[j] = vin[msiglist[i] + k]*gain[i] + deltav[i];
		if (vout[j] > vmax[i]) {
		    (void)fprintf(stderr, "v[%d] = %d (out of range)\n",
				  i, vout[j]);
		    vmax[i] = vout[j];
		}
		else if (vout[j] < vmin[i]) {
		    (void)fprintf(stderr, "v[%d] = %d (out of range)\n",
				  i, vout[j]);
		    vmin[i] = vout[j];
		}
	      }
	    }
	    if (putvec(vout) < 0) break;
	}
      }
    }
    else if (fflag != 0 && gflag == 0) {	/* change frequency only */
	while (getvec(vin) >= nisig && (nsamp == -1L || nsamp-- > 0L)) {
	    if (++nsm >= spm) {
		nsm = 0L;
		(void)fprintf(stderr, ".");
		(void)fflush(stderr);
		if (++nminutes >= 60) {
		    nminutes = 0;
		    (void)fprintf(stderr, "\n");
		}
	    }
	    for (i = 0; i < nosig; i++)
		v[i] = vin[siglist[i]] + deltav[i];
	    while (ot <= it) {
		double x = (ot%n == 0) ? 1.0 : (double)(ot % n)/(double)n;
		for (i = 0; i < nosig; i++) {
		    vout[i] = vv[i] + x*(v[i]-vv[i]);
		    if (vout[i] > vmax[i]) {
			(void)fprintf(stderr, "v[%d] = %d (out of range)\n",
				      i, vout[i]);
			vmax[i] = vout[i];
		    }
		    else if (vout[i] < vmin[i]) {
			(void)fprintf(stderr, "v[%d] = %d (out of range)\n",
				      i, vout[i]);
			vmin[i] = vout[i];
		    }
		}
		if (putvec(vout) < 0) { nsamp = 0L; break; }
		ot += m;
	    }
	    for (i = 0; i < nosig; i++)
		vv[i] = v[i];
	    if (it > mn) { it -= mn; ot -= mn; }
	    it += n;
	}
    }
    else if (fflag != 0 && gflag != 0) {	/* change frequency and gain */
	while (getvec(vin) >= nisig && (nsamp == -1L || nsamp-- > 0L)) {
	    if (++nsm >= spm) {
		nsm = 0L;
		(void)fprintf(stderr, ".");
		(void)fflush(stderr);
		if (++nminutes >= 60) {
		    nminutes = 0;
		    (void)fprintf(stderr, "\n");
		}
	    }
	    for (i = 0; i < nosig; i++)
		v[i] = vin[siglist[i]]*gain[i] + deltav[i];
	    while (ot <= it) {
		double x = (ot%n == 0) ? 1.0 : (double)(ot % n)/(double)n;
		for (i = 0; i < nosig; i++) {
		    vout[i] = vv[i] + x*(v[i]-vv[i]);
		    if (vout[i] > vmax[i]) {
			(void)fprintf(stderr, "v[%d] = %d (out of range)\n",
				      i, vout[i]);
			vmax[i] = vout[i];
		    }
		    else if (vout[i] < vmin[i]) {
			(void)fprintf(stderr, "v[%d] = %d (out of range)\n",
				      i, vout[i]);
			vmin[i] = vout[i];
		    }
		}
		if (putvec(vout) < 0) { nsamp = 0L; break; }
		ot += m;
	    }
	    for (i = 0; i < nosig; i++)
		vv[i] = v[i];
	    if (it > mn) { it -= mn; ot -= mn; }
	    it += n;
	}
    }
    if (nminutes > 0) (void)fprintf(stderr, "\n");

    /* Generate a new header file, if so requested. */
    if (nrec) {
	char *info, xinfo[80];

	(void)setsampfreq((double)ofreq);
	if (btstring[0] == '[') {
	    btstring[strlen(btstring)-1] = '\0';  /* discard final ']' */
	    (void)setbasetime(btstring+1);
	}
	(void)newheader(nrec);

	/* Copy info strings from the input header file into the new one.
	   Suppress error messages from the WFDB library. */
	wfdbquiet();
	if (info = getinfo(irec))
	    do {
		(void)putinfo(info);
	    } while (info = getinfo((char *)NULL));

	/* Append additional info summarizing what xform has done. */
	(void)sprintf(xinfo, "Produced by %s from record %s, beginning at %s",
		pname, irec, startp);
	(void)putinfo(xinfo);
	for (i = 0; i < nosig; i++)
	    if (siglist[i] != i) {
		(void)sprintf(xinfo,
			      "record %s, signal %d <- record %s, signal %d",
			nrec, i, irec, siglist[i]);
		(void)putinfo(xinfo);
	    }
	wfdbverbose();
    }

    wfdbquit();
    exit(0);	/*NOTREACHED*/
}

int gcd(x, y)	/* greatest common divisor of x and y (Euclid's algorithm) */
int x, y;
{
    while (x != y) {
	if (x > y) x-=y;
	else y -= x;
    }
    return (x);
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
 "usage: %s -i IREC [OPTIONS ...]\n",
 "where IREC is the name of the record to be converted, and OPTIONS may",
 " include any of:",
 " -a ANNOTATOR [ANNOTATOR ...]  copy annotations for the specified ANNOTATOR",
 "              from IREC;  two or more ANNOTATORs may follow -a",
 " -f TIME     begin at specified time",
 " -h          print this usage summary",
 " -H          open the input record in `high resolution' mode",
 " -M          process multifrequency record without changing frequencies",
 " -n NREC     create a header file, using record name NREC and signal",
 "              descriptions from IREC",
 " -N NREC     as for -n, but copy signal descriptions from OREC",
 " -o OREC     produce output signal file(s) as specified by the header file",
 "              for record OREC",
 " -s SIGNAL [SIGNAL ...]  write only the specified signal(s)",
 " -t TIME     stop at specified time",
 "Unless you use `-o' to specify an *existing* header that describes the",
 "desired signal files, you will be asked for output specifications.  Use",
 "`-n' to name the record to be created.  Use `-s' to select a subset of the",
 "input signals, or to re-order them in the output file;  arguments that",
 "follow `-s' are *input* signal numbers (0-31).",    /* WFDB_MAXSIG-1 = 31 */
NULL
};

void help()
{
    int i;

    (void)fprintf(stderr, help_strings[0], pname);
    for (i = 1; help_strings[i] != NULL; i++)
	(void)fprintf(stderr, "%s\n", help_strings[i]);
}
