/* file: sample.c	G. Moody	10 January 1991
			Last revised:  30 November 2002

-------------------------------------------------------------------------------
sample: digitize or play back signals on a PC using a Microstar DAP board
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

`sample' generates database records from analog signals, and generates
analog signals from database records.

`sample' runs under MS-DOS on an ISA-bus (AT-bus) system that contains a
Microstar DAP 1200 or 2400 series board (available from Microstar
Laboratories, http://www.mstarlabs.com/).

To compile `sample' successfully, you will need Microsoft C, version 5.0 or
later, or Turbo C/C++, and four files supplied by Microstar with all versions
of the DAP 1200 and 2400 (Microstar's Advanced Development Toolkit is *not*
required in order to compile or use this program).  These files are `c_lib.c',
`clock.h', and `ioutil.h', which should be installed in a directory searched
by your compiler for `#include <...>' files such as `stdio.h';  and
`cdapl.lib', which should be installed in a directory searched by your
compiler's linker for libraries such as `wfdb.lib'.  (If you are using Microsoft
C, copy `cdapl5.lib' from the Microstar distribution diskettes into your
library directory and rename it `cdapl.lib'.)

If you have not already done so, you must also compile and install a
large memory model version of the WFDB library (see `makefile.dos' in the
`lib' directory).  Although the program compiles and links successfully using
the small memory model and Microstar's `cdaps5.lib', there is apparently a bug
in `cdaps5.lib' that wedges the DAP (a condition that can be remedied only
by power-cycling the system).

Compile this program using Microsoft C with the command:
    cl -Ox -Ml sample.c -link wfdbl cdapl
or using Turbo C/C++ with the command:
    tcc -O -A -ml sample.c wfdbl.lib cdapl.lib
or using Borland C/C++ with the command:
    bcc -O -A -ml sample.c wfdbl.lib cdapl.lib

Note that you may need to use `-I' and `-L' options to instruct your compiler
where to find `#include' files and libraries;  see your compiler manual for
further information.  The symbols DAPINPUTS, DAPOUTPUTS, and DBUFSIZE
may be redefined using command-line `-D' options with any of these compilers.
If you compile using Microsoft C/C++ 7.0 (or later) with the `-Za' (ANSI C)
option, add `oldnames' as a final command-line argument, to link a library
containing aliases for functions such as open(), which have been renamed in
the standard MSC 7.0 library to conform with ANSI namespace rules.

Refer to `../doc/wag-src/sample.1' for further information.  */

#include <stdio.h>
#include <fcntl.h>

#ifdef __STDC__
/* True for ANSI C.  Definitions in this section work around gratuitous
deletions of universally available functions and macros that have not been
blessed by the ANSI C standard. */

#if _MSC_VER >= 700
/* For Microsoft C/C++ version 7.0 with the -Za option. */
#define O_BINARY _O_BINARY
#define O_RDONLY _O_RDONLY
#define O_WRONLY _O_WRONLY
#endif

#ifdef __TURBOC__
/* For Turbo C/C++ with the -A option.  The definition below is equivalent to
the one in stdio.h that is censored in ANSI C mode. */
#ifndef fileno
#define fileno(file) ((file)->fd)
#endif
#endif
#endif

#include <wfdb/wfdb.h>
/* `c_lib.c' is supplied by Microstar;  it contains nested `#include's of
   `clock.h' and `ioutil.h', also supplied by Microstar. */
#include <c_lib.c>

/* Constants.  Any of these definitions may be overridden by command-line
definitions (using the `-D' option) if desired. */
#ifndef DAPINPUTS
# define DAPINPUTS (16)		/* number of single-ended analog inputs */
#endif

#ifndef DAPOUTPUTS
# define DAPOUTPUTS (2)		/* number of analog outputs */
#endif

#ifndef DBUFSIZE
# define DBUFSIZE (10240)	/* default size for sample buffer, in bytes */
#endif

static char answer[32];		/* buffer for user's typed responses */
static char chanspec[WFDB_MAXSIG][5];/* DAP channel specifications for each
				   signal */
static double clockfreq;	/* DAP basic clock frequency in Hz */
static int dap2400;		/* 0: using DAP 1200, 1: using DAP 2400 */
static char *daptype = "check";	/* DAP model (see set_params(), below) */
static double daplversion = 3.3;/* DAPL version number (can be determined by
				   set_params();  the default is given here) */
static double freq = 0.0;	/* sampling frequency for record to be
				   digitized or played back, in samples per
				   signal per second of the original analog
				   signal */
static char *from;		/* if not NULL, string representing starting
				   time for playback operation */
static double maxfreq, minfreq;	/* maximum and minimum aggregate sampling
				   frequencies obtainable */
static long nsamp;		/* number of samples (per signal) to be
				   digitized or played back;  if zero, I/O
				   continues until stopped by key command */
static int nsig;		/* number of signals to be digitized */
static int nosig = DAPOUTPUTS;	/* number of signals to be played back */
static char nrec[8];		/* name to be used for output record */
static int pflag;		/* 1: play back, -1: generate DAPL code for
				   playback, 0: do neither of the above */
static char *pname;		/* name of this program, extracted from
				   argv[0] */
static double rate = -1.0;	/* presentation rate relative to the original
				   analog signal */
static char record[WFDB_MAXRNL+1];/* record name of header file to be read */
static int rflag;		/* if non-zero, use `raw' format I/O */
static WFDB_Siginfo s[WFDB_MAXSIG];/* WFDB information structures for signals
				   to be read or written */
static int sflag = 1;		/* 1: digitize, -1: generate DAPL code for
				   digitizing, 0: do neither of the above */
static long ticks;		/* number of DAP clock ticks per sampling
				   interval */
static int vmax[WFDB_MAXSIG];	/* highest sample values expected/observed */
static int vmin[WFDB_MAXSIG];	/* lowest sample values expected/observed */
static int xclock;		/* if non-zero, use external clock */
static char *timecmd;		/* printf format string for printing the
				   argument of the DAP `time' command */
static char *to;		/* if not NULL, string representing the ending
				   time for digitization or playback */

void adinit(), adget(), adbget(), adquit(), dainit(), daput(), daquit(),
    playback(), sample(), finish_sample(), get_dapl_program();

main(argc, argv)
int argc;
char *argv[];
{
    char chantype, *p;
    int channel, i;
    double fmin, fmax, ofreq;
    static char directory[32];		/* signal file name prefix */
    static char filename[WFDB_MAXSIG][32];	/* signal file names */
    static char description[WFDB_MAXSIG][32];/* signal descriptions */
    static char units[WFDB_MAXSIG][22];	/* signal units */

    /* Read the command line. */
    init(argc, argv);

    /* Set buffer sizes. */
    setibsize(DBUFSIZE);
    setobsize(DBUFSIZE);

    /* Initialize sample value limit arrays. */
    for (i = 0; i < WFDB_MAXSIG; i++) {
	vmax[i] = 2040;
	vmin[i] = -2040;
    }

    /* Get and validate the output record name. */
    while (sflag && (nrec[0] == '\0' || newheader(nrec) < 0)) {
        fprintf(stderr, "Choose a record name (up to 6 characters): ");
        fgets(nrec, 8, stdin); nrec[strlen(nrec)-1] = '\0';
    }

    /* Get and validate the presentation rate. */
    if (rate < 0.) {
	if (sflag) {
	    fprintf(stderr,
	       "\nThe presentation rate for live (not previously recorded)\n");
	    fprintf(stderr,
	       "signals is 1.  For recorded signals, the presentation rate\n");
	    fprintf(stderr,
		"is the quotient of the playback speed and the recording\n");
	    fprintf(stderr, "speed (often greater than 1).\n");
	}
	else {
	    fprintf(stderr,
	      "The presentation rate specifies the playback speed relative\n");
	    fprintf(stderr,
	      "to real time for the original analog signals.\n");
	}
	do {
	    fprintf(stderr, "Presentation rate (> 0) [1]: ");
	    fgets(answer, 32, stdin);
	    if (answer[0] == '\n') rate = 1.0;
	    else sscanf(answer, "%lf", &rate);
	} while (rate <= 0.0);
	fprintf(stderr, "\n");
    }

    /* If a prototype header was supplied with -o, read its signal
       specifications. */
    if (sflag && record[0] != '\0') {
	char *fmt, *dp;

	nsig = (DAPINPUTS < WFDB_MAXSIG) ? DAPINPUTS : WFDB_MAXSIG;
	if ((nsig = isigopen(record, s, -nsig)) < 0) exit(1);
	if (nsig == 0) {
	    fprintf(stderr,
		"%s: header for record %s contains no signal specifications\n",
		    pname, record);
	    exit(1);
	}
	freq = sampfreq(record);
	ticks = (long)(clockfreq/(freq*rate*nsig) + 0.5);
	if (to) nsamp = strtim(to);
	else nsamp = s[0].nsamp;
	fmt = (nsig <= DAPINPUTS/2) ? "D%d" : "S%d";
	for (dp = s[0].fname + strlen(s[0].fname) - 1; dp > s[0].fname; dp--)
	    if (*dp == '/' || *dp == ':' || *dp == '\\')
		break;
	if (s[0].fname < dp && dp <= s[0].fname + 30)
	    strncpy(directory, s[0].fname, dp - s[0].fname + 1);
	if (s[0].fmt == 16 && s[0].adcres == 16)
	    rflag = 1;
	if (rflag || nsig < 2 || s[1].group == 0)
	    sprintf(filename[0], "%s%s.dat", directory, nrec);
	for (i = 0; i < nsig; i++) {
	    sprintf(chanspec[i], fmt, i);
	    if (rflag || nsig < 2 || s[1].group == 0) {
		s[i].fname = filename[0];
		s[i].group = 0;
		s[i].fmt = s[0].fmt;
		s[i].bsize = s[0].bsize;
	    }
	    else {
		sprintf(filename[i], "%s%s.d%02d", directory, nrec, i);
		s[i].fname = filename[i];
		s[i].group = i;
	    }
	}
    }

    /* Otherwise, if a header of an existing record was supplied with -i, read
       its signal specifications. */
    else if (pflag && record[0] != '\0') {
	long t0 = 0L, t1 = 0L;

	if ((nsig = isigopen(record, s, WFDB_MAXSIG)) < 0) exit(1);
	if (nsig == 0) {
	    fprintf(stderr,
		"%s: header for record %s contains no signal specifications\n",
		    pname, record);
	    exit(1);
	}
	if (nosig > nsig) nosig = nsig;

	freq = sampfreq(record);
	ticks = (long)(clockfreq/(freq*rate*nosig) + 0.5);
	if (from) {
	    t0 = strtim(from);
	    if (t0 < 0L) t0 = -t0;
	    if (isigsettime(t0) < 0) exit(1);
	}
	if (to) {
	    t1 = strtim(to);
	    if (t1 < 0L) t1 = -t1;
	    else if (t1 == 0L) t1 = s[0].nsamp;
	}
	if (t1 > 0L) {
	    if (t1 <= t0) {
		fprintf(stderr,
			"%s: `to' time must be later than `from' time\n",
			pname);
		exit(1);
	    }
	    nsamp = t1 - t0;
	}
	else {
	    nsamp = s[0].nsamp;
	    if (nsamp > t0) nsamp -= t0;
	    else if (nsamp > 0L && t0 >= nsamp) {
		fprintf(stderr,
			"%s: `from' time must precede the end of the record\n",
			pname);
		exit(1);
	    }
	}
	if (s[0].fmt == 16 && s[0].adcres == 16 &&
	    s[nosig-1].group == s[0].group)
	    rflag = 1;

	for (i = 0; i < nosig; i++)
	    sprintf(chanspec[i], "A%d", i);
    }

    /* Otherwise, get sampling specifications interactively. */
    else {
	do {
	    fprintf(stderr, "Number of signals to be recorded (1-%d) [1]: ",
		    WFDB_MAXSIG);
	    fgets(answer, 32, stdin);
	    if (answer[0] == '\n') nsig = 1;
	    else sscanf(answer, "%d", &nsig);
	} while (nsig < 1 || nsig > WFDB_MAXSIG);
	fprintf(stderr, "\n");

	fmin = minfreq/(rate*nsig);
	fmax = maxfreq/(rate*nsig);
	freq = (WFDB_DEFFREQ < fmax) ? WFDB_DEFFREQ : fmax;

	if (rate != 1.0) {
	    fprintf(stderr,
		    "The sampling frequency is defined relative to real\n");
	    fprintf(stderr,
		    "time for the original signal%s signal will be\n",
		    nsig > 1 ? "s.  Each" : ".  The");
	    fprintf(stderr,
		    "sampled at %g (the presentation rate) times the\n", rate);
	    fprintf(stderr, "sampling frequency that you specify.\n");
	}
	do {
	    fprintf(stderr,
		    "Sampling frequency (Hz per signal, %g - %g) [%g]: ",
		   fmin, fmax, freq);
	    fgets(answer, 32, stdin);
	    if (answer[0] != '\n') sscanf(answer, "%lf", &freq);
	    if (fmin <= freq && freq <= fmax) {
		ticks = (long)(clockfreq/(freq*rate*nsig) + 0.5);
		fprintf(stderr,
		       "Using the on-board timer, %s signal can be sampled at",
		       nsig > 1 ? "each" : "the");
		fprintf(stderr, " %g Hz.\n",
			ofreq = clockfreq/(rate*nsig*ticks));
		if (dap2400) {
		    fprintf(stderr, 
		       "Alternatively, %s signal can be sampled at %g Hz\n",
		       nsig > 1 ? "each" : "the", freq);
		    fprintf(stderr, 
		       "using an external clock at %g * %g * %d (= %g) Hz.\n",
		       freq, rate, nsig, freq*rate*nsig);
		    fprintf(stderr, 
	        "Do you prefer to use an external clock at %g Hz (y/n)? [n]: ",
		       freq*rate*nsig);
		    fgets(answer, 32, stdin);
		    if (answer[0] == 'y' || answer[0] == 'Y') {
			xclock = 1;
			continue;
		    }
		}
		if (!xclock && freq != ofreq) {
		    fprintf(stderr,
			   "Is the frequency of %g Hz acceptable (y/n)? [y]: ",
			   ofreq);
		    fgets(answer, 32, stdin);
		    if (answer[0] != 'n' && answer[0] != 'N')
			freq = ofreq;
		    else
			freq = -1.0;
		}
	    }
	} while (freq < fmin || freq > fmax || setsampfreq(freq) < 0);
	fprintf(stderr, "\n");

	fprintf(stderr, "Specify the length of the record to be sampled\n");
	if (rate != 1.0)
	    fprintf(stderr,
		    "(in terms of real time for the original signals)\n");
	fprintf(stderr,
		"or press ENTER if sampling is to be stopped by keyboard\n");
	fprintf(stderr,
		"input after an indefinite period.\n");
	do {
	    fprintf(stderr, "Length of record (H:M:S): ");
	    fgets(answer, 32, stdin);
	} while (answer[0] != '\n' && (nsamp = strtim(answer)) < 1L);
	if (answer[0] == '\n') nsamp = 0L;
	fprintf(stderr, "\n");

	fprintf(stderr, "These signal file formats are supported:\n");
	fprintf(stderr, "  8  (8-bit first differences)\n");
	fprintf(stderr, "212  (12-bit amplitudes, bit-packed in pairs)\n");
	fprintf(stderr," 16  (12-bit amplitudes, sign-extended to 16 bits)\n");
	fprintf(stderr, "raw  (12-bit samples in DAP format 16-bit words)\n");
	rflag = (freq*rate*nsig > 20000.0) ? 1 : 0;
	do {
	    fprintf(stderr, "Select a format (8/212/16/raw) [%s]: ",
		    rflag ? "raw" : "16");
	    fgets(answer, 32, stdin);
	    if (answer[0] == '\n') s[0].fmt = 16;
	    else if (strncmp(answer, "raw", 3) == 0) {
		s[0].fmt = 16;
		rflag = 1;
	    }
	    else s[0].fmt = atoi(answer);
	} while (s[0].fmt != 16 && s[0].fmt != 212 && s[0].fmt != 8);
	fprintf(stderr, "\n");

	if (nsamp > 0L) {
	    long nb;

	    switch (s[0].fmt) {
	      case 8:	nb = nsamp * (long)nsig; break;
	      case 212:	nb = nsamp * (long)nsig * 1.5 + 0.5; break;
	      case 16:	nb = nsamp * (long)nsig * 2L; break;
	    }
	    fprintf(stderr,
		    "The sampled signal%s will require %ld bytes.\n",
		    nsig > 1 ? "s" : "", nb);
	}
	fprintf(stderr,
		"To specify the directory in which signal files are\n");
	fprintf(stderr,
		"to be written, type its name (with drive specification\n");
	fprintf(stderr,
		"if necessary, up to 30 characters), or press ENTER to\n");
	fprintf(stderr,
		"use the current directory.\n");
       	fprintf(stderr,
		"Directory for signal files: ");
	fgets(directory, 31, stdin);
	fprintf(stderr, "\n");

	/* Stamp out ugly MS-DOS `\' separators!  (Actually, this *does* serve
	   a purpose:  since the UNIX `/' separators are understandable to both
	   MS-DOS and UNIX versions of the WFDB library, this permits the
	   header file to be used on UNIX without modification, provided that
	   the directory structure is maintained.) */
	for (p = directory; *p != '\n'; p++)
	    if (*p =='\\') *p = '/';
	if (p > directory && *(p-1) != '/' && *(p-1) != ':')
	    *p++ = '/';
	*p = '\0';

	if (nsig > 1 && !rflag) {
	    fprintf(stderr, "Save all signals in one file (y/n)? [y]: ");
	    fgets(answer, 32, stdin);
	    fprintf(stderr, "\n");
	}
	if (nsig < 2 || rflag || (answer[0] != 'n' && answer[0] != 'N')) {
	    sprintf(filename[0], "%s%s.dat", directory, nrec);
	    for (i = 0; i < nsig; i++) {
		s[i].fname = filename[0];
		s[i].group = 0;
	    }
	}
	else {
	    for (i = 0; i < nsig; i++) {
		sprintf(filename[i], "%s%s.d%02d", directory, nrec, i);
		s[i].fname = filename[i];
		s[i].group = i;
	    }
	}

	fprintf(stderr, "For each signal to be digitized, specify:\n");
	fprintf(stderr, " - the input channel (one of `D0' through `D%d',\n",
		DAPINPUTS/2 - 1);
	fprintf(stderr,"   `S0' through `S%d', `B', or `G')\n", DAPINPUTS - 1);
	fprintf(stderr, " - a description (up to 30 characters)\n");
	fprintf(stderr, " - the physical units (up to 20 characters)\n");
	fprintf(stderr, " - the gain in adu/physical unit (0 if the signal\n");
	fprintf(stderr, "   is uncalibrated)\n");
	chantype = (nsig > DAPINPUTS/2) ? 'S' : 'D';
	s[0].gain = 0.0;
	for (i = 0; i < nsig; i++) {
	    s[i].fmt = s[0].fmt;
	    s[i].adcres = rflag ? 16: 12;
	    s[i].adczero = s[i].bsize = 0;
	    do {
		fprintf(stderr, "Signal %d input channel [%c%d]: ",
		       i, chantype, channel = i);
		fgets(answer, 32, stdin);
		if (answer[0] != '\n')
		    sscanf(answer, "%c%d", &chantype, &channel);
		switch (chantype) {
		  case 'b':		/* binary (digital) inputs */
		  case 'B':
		    sprintf(chanspec[i], "B");
		    /* This doesn't allow for the possibility that a digital
		       expansion board has been installed;  if so, it would
		       be necessary to add a digit (0-3) to specify which
		       set of 16 digital inputs are to be read. */
		    chantype = 'D';  /* assume next input is differential */
		    break;
		  case 'd':
		    chantype = 'D';
		  case 'D':		/* differential input */
		    if (0 <= channel && channel <= DAPINPUTS/2 - 1)
			sprintf(chanspec[i], "D%d", channel);
		    else
			channel = -1;
		    break;
		  case 'g':
		  case 'G':		/* ground */
		    sprintf(chanspec[i], "G");
		    chantype = 'S';	/* assume next signal single-ended */
		    break;
		  case 's':		/* single-ended input */
		    chantype = 'S';
		  case 'S':
		    if (0 <= channel && channel <= DAPINPUTS - 1)
			sprintf(chanspec[i], "S%d", channel);
		    else
			channel = -1;
		    break;
		  default:
		    chantype = 'D';
		    channel = -1;
		    break;
		}
	    } while (channel < 0);
	    fprintf(stderr, "Signal %d description [none]: ",i);
	    fgets(description[i], 32, stdin);
	    description[i][strlen(description[i])-1] = '\0';
	    s[i].desc = description[i];
	    fprintf(stderr, "Signal %d units [%s]: ", i,
		    (i > 0) ? s[i-1].units : "mV");
	    fgets(units[i], 20, stdin);
	    if (units[i][0] == '\n') s[i].units = "mV";
	    else
		for (p = s[i].units = units[i]; *p; p++) {
		    if (*p == ' ' || *p == '\t') *p = '_';
		    else if (*p == '\n') { *p = '\0'; break; }
		}
	    if (i > 0) s[i].gain = s[i-1].gain;
	    fprintf(stderr, "Signal %d gain (adu/%s) [%g]: ",
		    i, s[i].units, s[i].gain);
	    fgets(answer, 32, stdin);
	    if (answer[0] != '\n')
		sscanf(answer, "%lf", &s[i].gain);
	}
	fprintf(stderr, "\n");
    }

    get_dapl_program();

    /* Adjust number of samples and frequency to account for finite resolution
       of internal clock.  Note that the adjustment of nsamp is appropriate
       if the duration was specified in time units, but may not be so if the
       the duration was specified directly as a number of samples. */
    if (!pflag && !xclock && freq != clockfreq/(rate*nsig*ticks)) {
	char ttime[20];

	strcpy(ttime, mstimstr(nsamp));
	freq = clockfreq/(rate*nsig*ticks);
	setsampfreq(0.0);	/* avoid error message from next setsampfreq */
	setsampfreq(freq);
	nsamp = strtim(ttime);
    }

    if (pflag == 1)
	playback();
    else if (sflag == 1)
	sample();
    else
	finish_sample();
    exit(0);
}

static int cpr = 1;		/* compression ratio (number of samples
				   read/number of samples written) */
static FILE *dapfile;		/* if not NULL, DAPL program file pointer */
static char *dapfname;		/* if not NULL, name of DAPL program file */
static int iflag = 1;		/* if non-zero, prompt to begin digitization
				   or playback */

init(argc, argv)
int argc;
char *argv[];
{
    int i;

    /* Extract the name of this program from argv[0]. */
    for (pname = argv[0] + strlen(argv[0]) - 1; pname > argv[0]; pname--) {
	if (*pname == '.') *pname = '\0';
	else if ('A' <= *pname && *pname <= 'Z') *pname += 'a' - 'A';
	else if (*pname == '\\' || *pname == ':') {
	    pname++;
	    break;
	}
    }

    /* Read and interpret command-line arguments. */
    for (i = 1; i < argc; i++) {
	if (*argv[i] == '-') switch (*(argv[i]+1)) {
	  case '1':	/* single-channel playback */
	    nosig = 1;
	    break;
	  case 'b':	/* batch mode */
	    iflag = 0;
	    break;
	  case 'd':	/* device type follows */
	    if (++i >= argc) {
		fprintf(stderr, "%s: device type must follow -d\n", pname);
		exit(1);
	    }
	    daptype = argv[i];
	    break;
	  case 'f':	/* start time follows */
	    if (++i >= argc) {
		fprintf(stderr, "%s: start time must follow -f\n", pname);
		exit(1);
	    }
	    from = argv[i];
	    break;
	  case 'h':	/* help requested */
	    fprintf(stderr, "usage: %s [ options ]\n", pname);
	    fprintf(stderr, "options:\n");
	    fprintf(stderr, " -b\t\tbatch mode\n");
	    fprintf(stderr,
		    " -d TYPE\tspecify device type (-d help for list)\n");
	    fprintf(stderr, " -f TIME\tbegin at TIME (only with -i)\n");
	    fprintf(stderr, " -h\t\tprint (this) help\n");
	    fprintf(stderr,
		    " -i RECORD\tplay back RECORD (not with -n, -N, -o)\n");
	    fprintf(stderr,
		    " -n RECORD\tcreate RECORD (signals and header)\n");
	    fprintf(stderr, " -N RECORD\tcreate header (only) for RECORD\n");
	    fprintf(stderr,
	     " -o RECORD\tdigitize using specs from existing RECORD header\n");
	    fprintf(stderr, " -p FILE\tuse the DAPL program in FILE\n");
	    fprintf(stderr,
		    " -s N\t\trecord or play back 1 of every N samples\n");
	    fprintf(stderr, " -t TIME\tstop at TIME\n");
	    fprintf(stderr,
		    " -x N\t\toperate at N times real time (not with -p)\n");
	    fprintf(stderr, " -1\t\tsingle-channel playback mode (with -i)\n");
	    fprintf(stderr,
		    "Type `%s' without any options to enter specifications\n",
		    pname);
	    fprintf(stderr,
		    "interactively.\n");
	    exit(1);
	  case 'i':	/* name of record to be played back follows */
	    if (++i >= argc) {
		fprintf(stderr, "%s: record name must follow -i\n", pname);
		exit(1);
	    }
	    strncpy(record, argv[i], 7);
	    pflag = 1; sflag = 0;
	    break;
	  case 'n':	/* name of record to be created follows */
	    if (++i >= argc) {
		fprintf(stderr, "%s: record name must follow -n\n", pname);
		exit(1);
	    }
	    strncpy(nrec, argv[i], 7);
	    break;
	  case 'N':	/* name of prototype header to be written follows */
	    if (++i >= argc) {
		fprintf(stderr, "%s: record name must follow -N\n", pname);
		exit(1);
	    }
	    strncpy(nrec, argv[i], 7);
	    sflag = -1;
	    break;
	  case 'o':	/* name of prototype header to be read follows */
	    if (++i >= argc) {
		fprintf(stderr, "%s: record name must follow -o\n", pname);
		exit(1);
	    }
	    strncpy(record, argv[i], 7);
	    break;
	  case 'p':	/* name of DAPL program file follows */
	    if (++i >= argc) {
		fprintf(stderr, "%s: name of DAPL file must follow -p\n",
			pname);
		exit(1);
	    }
	    dapfname = argv[i];
	    break;
	  case 's':	/* compression ratio follows */
	    if (++i >= argc) {
		fprintf(stderr, "%s: compression ratio must follow -s\n",
			pname);
		exit(1);
	    }
	    cpr = atoi(argv[i]);
	    if (cpr < 1) cpr = 1;
	    break;
	  case 't':	/* stop time follows */
	    if (++i >= argc) {
		fprintf(stderr, "%s: stop time must follow -t\n", pname);
		exit(1);
	    }
	    to = argv[i];
	    break;
	  case 'x':	/* presentation rate follows */
	    if (++i >= argc) {
		fprintf(stderr, "%s: presentation rate must follow -x\n",
			pname);
		exit(1);
	    }
	    rate = (double)atof(argv[i]);
	    break;
	  default:
	    fprintf(stderr, "%s: unrecognized option %s\n", pname, argv[i]);
	    exit(1);
	}
	else {
	    fprintf(stderr, "%s: unrecognized argument %s\n", pname, argv[i]);
	    exit(1);
	}
    }

    /* Set device-dependent parameters. */
    if (set_params() == 0)
	exit(1);

    if (dapfname && (dapfile = fopen(dapfname, "rt")) == NULL) {
	fprintf(stderr, "%s: can't open DAPL program file %s\n", pname,
		dapfname);
	exit(1);
    }
}

/* Root and pointer to singly-linked list of DAPL instructions. */
struct dapline {
    char *pline;
    struct dapline *next;
} dapl0, *dp = &dapl0;

/* Add a DAPL line to the instruction list. */
save_dapl_line(s)
char *s;
{
    if (dp->pline = malloc(strlen(s)+1)) {
	strcpy(dp->pline, s);
	if (dp->next = malloc(sizeof(struct dapline))) {
	    dp = dp->next;
	    dp->pline = NULL;
	    dp->next = NULL;
	}
    }
}

/* Construct the DAPL instruction list, either from a DAPL program file,
   the info strings of the header, or from scratch. */
void get_dapl_program()
{
    static char buf[80], *p;
    int i;

    if (dapfile) {
	if (fgets(buf, 80, dapfile)) {
	    if (strncmp("; start ", buf, 8))
		save_dapl_line("; start a");
	}
	do {
	    buf[strlen(buf)-1] = '\0';	/* strip off newline */
	    if (strcmp(buf, "clock external") == 0) xclock = 1;
	    save_dapl_line(buf);
	} while (fgets(buf, 80, dapfile));
	fclose(dapfile);
    }
    if (dp == &dapl0 && record[0] != '\0') {
	setsampfreq(0.0);
	if (sflag) {
	    if (p = getinfo(record))
		do {
		    if (*p == '>') {
			if (dp == &dapl0 && strncmp("; start ", p+1, 8))
			    save_dapl_line("; start a");
			if (strcmp(p+1, "clock external") == 0) xclock = 1;
			save_dapl_line(p+1);
		    }
		} while (getinfo(NULL));
	}
	else {
	    if (p = getinfo(record))
		do {
		    if (*p == '<') {
			if (dp == &dapl0 && strncmp("; start ", p+1, 8))
			    save_dapl_line("; start a");
			if (strcmp(p+1, "clock external") == 0) xclock = 1;
			save_dapl_line(p+1);
		    }
		} while (getinfo(NULL));
	}
	setsampfreq(freq);
    }
    if (dp == &dapl0) {
	if (sflag) {
	    save_dapl_line("; start a");
	    save_dapl_line("reset");
	    sprintf(buf, "idef a %d", nsig);
	    save_dapl_line(buf);
	    if (xclock)
		save_dapl_line("clock external");
	    for (i = 0; i < nsig; i++) {
		sprintf(buf, "set %d %s", i, chanspec[i]);
		save_dapl_line(buf);
	    }
	    sprintf(buf, timecmd, ticks*1.0e6/clockfreq);
	    save_dapl_line(buf);
	    save_dapl_line("bprint");
	    save_dapl_line("end");
	}
	else {		/* generate DAPL for playback */
	    if (!dap2400) {
		fprintf(stderr,
			"%s: can't generate playback program for DAP 1200\n",
			pname);
		exit(1);
	    }
	    if (nosig > 1 && daplversion < 3.3) {
		fprintf(stderr,
    "%s: can't generate multichannel playback program for DAPL version %g\n",
			pname, daplversion);
		fprintf(stderr, "(DAPL version 3.3 or later needed)\n");
		exit(1);
	    }
	    if (nosig == 1)
		save_dapl_line("; start a");
	    else
		save_dapl_line("; start a,b");
	    save_dapl_line("reset");
	    if (nosig == 1) {
		save_dapl_line("odef a 1");
		save_dapl_line("set 0 A0");
	    }
	    else {
		for (i = 0; i < nosig; i++) {
		    sprintf(buf, "pipe p%d", i);
		    save_dapl_line(buf);
		}
		save_dapl_line("pdef a");
		sprintf(buf, "separate($binin,p0,p1");
		for (i = 2; i < nosig; i++) {
		    char buf2[6];
		    sprintf(buf2, ",p%d", i);
		    strcat(buf, buf2);
		}
		strcat(buf, ")");
		save_dapl_line(buf);
		save_dapl_line("end");
		sprintf(buf, "odef b %d", nosig);
		save_dapl_line(buf);
		if (xclock) save_dapl_line("clock external");
		for (i = 0; i < nosig; i++) {
		    sprintf(buf, "set %d A%d", i, i);
		    save_dapl_line(buf);
		}
	    }
	    sprintf(buf, timecmd, ticks*1.0e6/clockfreq);
	    save_dapl_line(buf);
	    if (nosig == 1)
		save_dapl_line("direct($binin,0)");
	    else {
		for (i = 0; i < nosig; i++) {
		    sprintf(buf, "direct(p%d,%d)", i, i);
		    save_dapl_line(buf);
		}
	    }
	    save_dapl_line("end");
	}
    }
}

/* Save the current DAPL program as info strings in the new header file. */
void put_dapl_program()
{
    char buf[81];

    if (sflag) buf[0] = '>';
    else buf[0] = '<';
    for (dp = &dapl0; dp->pline; dp = dp->next) {
	strncpy(buf+1, dp->pline, 80);
	putinfo(buf);
    }
}

int dapofile;

void playback()
{
    char *p, *vbuf, *ve;
    int ifile, n, v[WFDB_MAXSIG];
    long t = 0L, tt = 0L, start_time, end_time;
    unsigned r_interval;

    /* Arrange for progress reports about once per second of elapsed time. */
    r_interval = rflag ? DBUFSIZE : (unsigned)strtim("1");
    if (nsamp > 0L && r_interval > nsamp) r_interval = (unsigned)nsamp;

    if ((dapofile = open("ACCEL1", O_WRONLY | O_BINARY)) == -1) {
	fprintf(stderr, "%s: can't open ACCEL1\n", pname);
	exit(1);
    }
    binary(dapofile);

    /* If using raw mode, close and reopen the signal file. */
    if (rflag) {
	wfdbquit();
	setsampfreq(freq*nosig);
	if ((vbuf = (char *)malloc(2*r_interval)) == NULL) {
	    fprintf(stderr, "%s: insufficient memory\n", pname);
	    exit(1);
	}
	if ((ifile = open(s[0].fname, O_RDONLY | O_BINARY)) == -1) {
	    fprintf(stderr, "%s: can't read `%s'\n", pname, s[0].fname);
	    exit(1);
	}
    }

    if (iflag) {
	fprintf(stderr, "To begin playback, press ENTER: ");
	fgets(answer, 32, stdin);
    }

    if (nsamp) {
	dainit();
	time(&start_time);
	if (rflag) {
	    ve = vbuf + 2*r_interval;
	    while (t < nsamp) {
		/* Fill the buffer with samples from the signal file. */
		for (p = vbuf; p < ve && (n = read(ifile,p,ve-p)) > 0; p += n)
		    ;
		if (n < 0) {
		    fprintf(stderr, "\n%s: end of file in `%s'\n", pname,
			    s[0].fname);
		    p -= n;
		    nsamp = t;	/* arrange for exit at end of this iteration */
		}
		if (write(dapofile, vbuf, (int)(p-vbuf)) < 0) {
		    fprintf(stderr, "\n%s: error writing samples to DAP\n",
			    pname);
		    daquit();
		    exit(1);
		}
		t += r_interval;
		fprintf(stderr, "%s\r", timstr(t));
		if (nsamp - t < r_interval) {
		    r_interval = (unsigned)(nsamp - t);
		    ve = vbuf + 2*r_interval;
		}
	    }
	}
	else {
	    while (t < nsamp) {
		if (getvec(v) < nosig) break;
		daput(v);
		if (++tt >= r_interval) {
		    t += tt;
		    fprintf(stderr, "%s\r", timstr(t));
		    tt = 0;
		    if (nsamp - t < r_interval)
			r_interval = (unsigned)(nsamp - t);
		}
	    }
	}
    }
    else {
	fprintf(stderr, "Press any key to stop playback\n");
	dainit();
	time(&start_time);
	if (rflag) {
	    ve = vbuf + 2*r_interval;
	    do {
		/* Fill the buffer with samples from the signal file. */
		for (p = vbuf; p < ve && (n = read(ifile,p,ve-p)) > 0; p += n)
		    ;
		if (n < 0) {
		    fprintf(stderr, "\n%s: end of file in `%s'\n", pname,
			    s[0].fname);
		    p -= n;
		}
		if (write(dapofile, vbuf, (int)(p-vbuf)) < 0) {
		    fprintf(stderr, "\n%s: error writing samples to DAP\n",
			    pname);
		    daquit();
		    exit(1);
		}
		t += r_interval;
		fprintf(stderr, "%s\r", timstr(t));
	    } while (n > 0 && !kbhit());
	}
	else {
	    while (getvec(v) >= nosig) {
		daput(v);
		if (++tt >= r_interval) {
		    t += tt;
		    fprintf(stderr, "%s\r", timstr(t));
		    if (kbhit()) break;
		    tt = 0;
		}
	    }
	}
    }
    /* Under a multitasking OS, it would be better to use something akin
       to the UNIX sleep() system call to wait, rather than the busy-waiting
       loop below.  Under MS-DOS, this doesn't matter;  under MS Windows or
       DesqView, you may want to try an alternative.  Don't just remove the
       loop below, however;  it's needed to allow the DAP time to finish
       its output before turning it off. */
    do {
	time(&end_time);
    } while ((long)(end_time - start_time) < (long)(t/(freq*rate) + 1));
    daquit();

    printf("\nElapsed time: %ld seconds\n", (long)(end_time - start_time));
    if (!rflag)
	wfdbquit();
}

static int bflag;		/* if non-zero, use `adbget' to read bipolar
				   input from DAP 1200 */

void sample()
{
    char *p, *vbuf, *ve;
    int ifile, n, ofile, v[WFDB_MAXSIG];
    long t = 0L, tt = 0L, start_time, end_time;
    unsigned r_interval;

    if (osigfopen(s, nsig) < nsig) exit(1);

    /* Arrange for progress reports about once per second (or once per buffer
       if using raw mode). */
    r_interval = rflag ? DBUFSIZE : (unsigned)strtim("1");
    if (nsamp > 0L && r_interval > nsamp) r_interval = (unsigned)nsamp;

    /* If using raw mode, write the header now, then close and reopen the
       signal file. */
    if (rflag) {
	finish_sample();
	setsampfreq(freq*nsig);
	if ((vbuf = (char *)malloc(2*r_interval)) == NULL) {
	    fprintf(stderr, "%s: insufficient memory\n", pname);
	    exit(1);
	}
	if ((ifile = open("ACCEL1", O_RDONLY | O_BINARY)) == -1) {
	    fprintf(stderr, "%s: can't open ACCEL1\n", pname);
	    exit(1);
	}
	binary(ifile);
	if ((ofile = open(s[0].fname, O_WRONLY | O_BINARY)) == -1) {
	    fprintf(stderr, "%s: can't create `%s'\n", pname, s[0].fname);
	    exit(1);
	}
    }

    if (iflag) {
	fprintf(stderr, "To begin sampling, press ENTER;  to specify a\n");
	fprintf(stderr, " start time other than the current time, enter\n");
	fprintf(stderr, " it in H:M:S format before pressing ENTER: ");
	fgets(answer, 32, stdin); answer[strlen(answer)-1] = '\0';
	fprintf(stderr, "\n");
	setbasetime(answer);
    }
    else
	setbasetime("");

    if (nsamp) {
	adinit();
	time(&start_time);
	if (rflag) {
	    ve = vbuf + 2*r_interval;
	    while (t < nsamp) {
		/* Fill the buffer with samples from the DAP. */
		for (p = vbuf; p < ve && (n = read(ifile,p,ve-p)) > 0; p += n)
		    ;		
		if (n < 0) {
		    fprintf(stderr, "\n%s: error reading samples from DAP\n",
			    pname);
		    adquit();
		    exit(1);
		}
		if (write(ofile, vbuf, (int)(p-vbuf)) < 0) {
		    fprintf(stderr, "\n%s: write error\n", pname);
		    adquit();
		    exit(1);
		}
		t += r_interval;
		fprintf(stderr, "%s\r", timstr(t));
		if (nsamp - t < r_interval) {
		    r_interval = (unsigned)(nsamp - t);
		    ve = vbuf + 2*r_interval;
		}
	    }
	}
	else if (bflag) {
	    while (t < nsamp) {
		adbget(v);
		if (putvec(v) < 0) break;
		if (++tt >= r_interval) {
		    t += tt;
		    fprintf(stderr, "%s\r", timstr(t));
		    tt = 0;
		    if (nsamp - t < r_interval)
			r_interval = (unsigned)(nsamp - t);
		}
	    }
	}
	else {
	    while (t < nsamp) {
		adget(v);
		if (putvec(v) < 0) break;
		if (++tt >= r_interval) {
		    t += tt;
		    fprintf(stderr, "%s\r", timstr(t));
		    tt = 0;
		    if (nsamp - t < r_interval)
			r_interval = (unsigned)(nsamp - t);
		}
	    }
	}
    }
    else {
	fprintf(stderr, "Press any key to stop sampling\n");
	adinit();
	time(&start_time);
	if (rflag) {
	    ve = vbuf + 2*r_interval;
	    do {
		/* Fill the buffer with samples from the DAP. */
		for (p = vbuf; p < ve && (n = read(ifile,p,ve-p)) > 0; p += n)
		    ;		
		if (n < 0) {
		    fprintf(stderr, "%s: error reading samples from DAP\n",
			    pname);
		    adquit();
		    exit(1);
		}
		if (write(ofile, vbuf, (int)(p-vbuf)) < 0) {
		    fprintf(stderr, "\n%s: write error\n", pname);
		    adquit();
		    exit(1);
		}
		t += r_interval;
		fprintf(stderr, "%s\r", timstr(t));
	    } while (!kbhit());
	}
	else if (bflag) {
	    do {
		adbget(v);
		if (++tt >= r_interval) {
		    t += tt;
		    fprintf(stderr, "%s\r", timstr(t));
		    if (kbhit()) break;
		    tt = 0;
		}
	    } while (putvec(v) >= 0);
	}
	else {
	    do {
		adget(v);
		if (++tt >= r_interval) {
		    t += tt;
		    fprintf(stderr, "%s\r", timstr(t));
		    if (kbhit()) break;
		    tt = 0;
		}
	    } while (putvec(v) >= 0);
	}
    }
    time(&end_time);
    adquit();

    printf("\nElapsed time: %ld seconds\n", (long)(end_time - start_time));
    if (rflag)
	close(ofile);
    else
	finish_sample();
}

void finish_sample()
{
    if (sflag == -1)
	(void)setheader(nrec, s, nsig);
    else
	(void)newheader(nrec);
    put_dapl_program();
    wfdbquit();
}

int set_params()
{
    if (strcmp(daptype, "check") == 0) {
	char m;
	int i, j;

	fprintf(stderr, "Testing DAP ... ");
	InitDap();
	fprintf(DapOut, "hello\n");
	fflush(DapOut);
	if (fscanf(DapIn, " *** DAPL Interpreter [%lf %c%d/%d]",
		   &daplversion, &m, &i, &j) < 4) {
	    fprintf(stderr, "\n%s: DAP does not respond.\n", pname);
	    exit(1);
	}
	LeaveDap();
	switch (100*i + j) {
	  case 102: daptype = "1200/2"; break;
	  case 103: daptype = "1200/3"; break;
	  case 104: daptype = "1200/4"; break;
	  case 204: daptype = "2400/4"; break;
	  case 205: daptype = "2400/5"; break;
	  case 206: daptype = "2400/6"; break;
	  default:  fprintf(stderr, "\n%s: unrecognized DAP type %c%d/%d\n",
			    m, i, j);
	            exit(1);
	}
	fprintf(stderr, "\rDAP model %s, DAPL version %g\n",
		daptype, daplversion);
    }
    if (strncmp(daptype, "1200/2", 6) == 0 ||
	strncmp(daptype, "1200/3", 6) == 0) {
	minfreq = 31.25;
	maxfreq = 50000.0;
	clockfreq = 2.0e6;
	timecmd = "time %.1lf";
	if (*(daptype + 6) == 'B') bflag = 1;
    }
    else if (strncmp(daptype, "1200/4", 6) == 0 ||
	     strcmp(daptype, "2400/4") == 0) {
	minfreq = 38.462;
	maxfreq = 156250.0;
	clockfreq = 2.5e6;
	timecmd = "time %.1lf";
	if (*daptype == '2') dap2400 = 1;
	else if (*(daptype + 6) == 'B') bflag = 1;
    }
    else if (strcmp(daptype, "2400/5") == 0 ||
	     strcmp(daptype, "2400/6") == 0) {
	minfreq = 62.5;
	maxfreq = 235295.0;
	clockfreq = 4.0e6;
	timecmd = "time %.2lf";
	dap2400 = 1;
    }
    else {
	if (strcmp(daptype, "help"))
	    fprintf(stderr,
		    "%s: unrecognized device type %s\n", pname, daptype);
	fprintf(stderr, "  Recognized types are:\n");
	fprintf(stderr, "    1200/2   1200/2B   2400/4\n");
	fprintf(stderr, "    1200/3   1200/3B   2400/5\n");
	fprintf(stderr, "    1200/4   1200/4B   2400/6\n");
	fprintf(stderr,
		"  If no `-d' option is given, `sample' determines the");
	fprintf(stderr, " type using the DAPL\n   `hello' command.\n");
	return (0);
    }
    return (1);
}

void adinit()
{
    InitDap();
    SetInput(DapIn, 1);
    FinFlush(DapIn);
    SetInput(DapIn, 0);
    StartCommands();

    /* Download the DAPL program. */
    for (dp = dapl0.next; dp->pline; dp = dp->next) {
	fputs(dp->pline, DapOut);
	fputs("\n", DapOut);
    }
    CheckErrors();
    FinFlush(DapIn);

    /* Send the `start' command (skipping the initial `; '). */
    fputs(dapl0.pline+2, DapOut);
    fputs("\n", DapOut);
    fflush(DapOut);
    if (!rflag)
	SetInput(DapIn, 1);
}

void dainit()
{
    adinit();
    SetOutput(DapOut, 1);
}

void adget(v)
int *v;
{
    int i;

    /* DAP 2400 series boards, and DAP 1200 series boards sampling unipolar
       inputs, return 12-bit samples left-justified in 16-bit words (the four
       low bits are all zero).  The `>> 4' operation within the loop below
       produces right-justified samples in the range of -2048 to +2047
       inclusive. */

    for (i = 0; i < nsig; i++) {
	*v = getw(DapIn) >> 4;
	if (*v < vmin[i])
	    fprintf(stderr, "\nsignal %d too low (%d)\n", i, vmin[i] = *v);
	else if (*v > vmax[i])
	    fprintf(stderr, "\nsignal %d too high (%d)\n", i, vmax[i] = *v);
	v++;
    }

    /* Get and discard additional samples as specified. */
    if (cpr > 1) {
	    int c;
	    
	    for (c = 1; c < cpr; c++)
		    for (i = 0; i < nsig; i++)
			    getw(DapIn);
	}
}

void adbget(v)
int *v;
{
    int i;

    /* This variant of `adget' is intended for use with the DAP 1200 if it
       has been configured for sampling bipolar inputs (see `Analog Signal
       Path Selection, DAP 1200' in Microstar's Hardware Manual, and `BPRINT'
       in Microstar's DAPL Manual).  This function has not been tested. */

    for (i = 0; i < nsig; i++) {
	*v = ((getw(DapIn) << 1) - 32768) >> 4;
	if (*v < vmin[i])
	    fprintf(stderr, "\nsignal %d too low (%d)\n", i, vmin[i] = *v);
	else if (*v > vmax[i])
	    fprintf(stderr, "\nsignal %d too high (%d)\n", i, vmax[i] = *v);
	v++;
    }

    if (cpr > 1) {
	int c;
	    
	for (c = 1; c < cpr; c++)
	    for (i = 0; i < nsig; i++)
		getw(DapIn);
    }
}

void daput(v)
int *v;
{
    int i, vout;

    for (i = 0; i < nosig; i++, v++) {
	vout = (*v) << 4;
	write(dapofile, &vout, 2);
	/* You would think that
	   putw((*v) << 4, DapOut);
	   would work here. (I did.) It doesn't -- at least not if v < 0. */
    }
    if (cpr > 1) {
	int c, v[WFDB_MAXSIG];

	for (c = 1; c < cpr && getvec(v) >= nosig; c++)
	    ;
    }
}

void adquit()
{
    if (!rflag)
	SetInput(DapIn, 0);
    fputs("stop system \n", DapOut);
    fflush(DapOut);
    LeaveDap();
}

void daquit()
{
    fflush(DapOut);
    SetOutput(DapOut, 0);
    adquit();
}
