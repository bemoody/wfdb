/* file: rdsamp.c	G. Moody	23 June 1983
			Last revised:   5 October 2001

-------------------------------------------------------------------------------
rdsamp: Print an arbitrary number of samples from each signal
Copyright (C) 2001 George B. Moody

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
#include <wfdb/wfdb.h>

/* Define the default WFDB path for CD-ROM versions of this program (the MS-DOS
   executables found in the `bin' directories of the various CD-ROMs).  This
   program has been revised since the appearance of these CD-ROMs; compiling
   this file will not produce executables identical to those on the CD-ROMs.
   Note that the drive letter is not included in these WFDB path definitions,
   since it varies among systems.
 */
#ifdef MITCDROM
#define WFDBP ";\\mitdb;\\nstdb;\\stdb;\\vfdb;\\afdb;\\cdb;\\svdb;\\ltdb;\\cudb"
#endif
#ifdef STTCDROM
#define WFDBP ";\\edb;\\valedb"
#endif
#ifdef SLPCDROM
#define WFDBP ";\\slpdb"
#endif
#ifdef MGHCDROM
#define WFDBP ";\\mghdb"
#endif

char *pname;

main(argc, argv)
int argc;
char *argv[];
{
    char *record = NULL, *prog_name();
    int highres = 0, i, isiglist, nsig, nosig = 0, pflag = 0, s, *sig = NULL,
	vflag = 0;
    long from = 0L, maxl = 0L, to = 0L;
    WFDB_Sample *v;
    WFDB_Siginfo *si;
    void help();

#ifdef WFDBP
    char *wfdbp = getwfdb();
    if (*wfdbp == '\0')
	setwfdb(WFDBP);
#endif

    pname = prog_name(argv[0]);

    /* Accept old syntax. */
    if (argc >= 2 && argv[1][0] != '-') {
	record = argv[1];
	i = 2;
	if (argc > 2 && argv[2][0] != '-') {
	    from = 2;
	    i = 3;
	}
	if (argc > 3 && argv[3][0] != '-') {
	    to = 3;
	    i = 4;
	}
	if (argc > 4) {
	    isiglist = 4;
	    while (i < argc && argv[i][0] != '-') {
		i++;
		nosig++;
	    }
	}
    }
    else
	i = 1;

    for ( ; i < argc; i++) {
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
	  case 'p':	/* output in physical units specified */
	    pflag = 1;
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
	  case 't':	/* end time */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: time must follow -t\n",pname);
		exit(1);
	    }
	    to = i;
	    break;
	  case 'v':	/* verbose output -- include column headings */
	    vflag = 1;
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
    if ((nsig = isigopen(record, NULL, 0)) <= 0) exit(2);
    if ((v = malloc(nsig * sizeof(WFDB_Sample))) == NULL ||
	(si = malloc(nsig * sizeof(WFDB_Siginfo))) == NULL) {
	(void)fprintf(stderr, "%s: insufficient memory\n", pname);
	exit(2);
    }
    if ((nsig = isigopen(record, si, nsig)) <= 0)
	exit(2);
    for (i = 0; i < nsig; i++)
	if (si[i].gain == 0.0) si[i].gain = WFDB_DEFGAIN;
    if (highres)
        setgvmode(WFDB_HIGHRES);
    if (from > 0L && (from = strtim(argv[from])) < 0L)
	from = -from;
    if (isigsettime(from) < 0)
	exit(2);
    if (to > 0L && (to = strtim(argv[to])) < 0L)
	to = -to;
    if (maxl > 0L && (maxl = strtim(argv[maxl])) < 0L)
	maxl = -maxl;
    if (maxl && (to == 0L || to > from + maxl))
	to = from + maxl;
    if (nosig) {		/* print samples only from specified signals */
#ifndef lint
	if ((sig = (int *)malloc((unsigned)nosig*sizeof(int))) == NULL) {
	    (void)fprintf(stderr, "%s: insufficient memory\n", pname);
	    exit(2);
	}
#endif
	for (i = 0; i < nosig; i++) {
	    if ((s = atoi(argv[isiglist+i])) < 0 || s >= nsig) {
		(void)fprintf(stderr, "%s: can't read signal %d\n", pname, s);
		exit(2);
	    }
	    sig[i] = s;
	}
	nsig = nosig;
    }
    else {			/* print samples from all signals */
#ifndef lint
	if ((sig = (int *)malloc((unsigned)nsig*sizeof(int))) == NULL) {
	    (void)fprintf(stderr, "%s: insufficient memory\n", pname);
	    exit(2);
	}
#endif
	for (i = 0; i < nsig; i++)
	    sig[i] = i;
    }

    /* Print column headers if '-v' option selected. */
    if (vflag) {
	char *p, *t;
	int l;

	if (pflag == 0) (void)printf("samp #");
	else (void)printf("time");
	if ((t = malloc((strlen(record)+30) * sizeof(char))) == NULL) {
	    fprintf(stderr, "%s: insufficient memory\n", pname);
	    exit(2);
	}
	for (i = 0; i < nsig; i++) {
	    (void)sprintf(t, "record %s, signal %d", record, sig[i]);
	    p = si[sig[i]].desc;
	    if (strcmp(p, t) == 0) {
		(void)sprintf(t, "sig %d", sig[i]);
		p = t;
	    }
	    l = strlen(p);
	    if (l > 7) p += l - 7;
	    /* Print the last 7 characters of each signal description. */
	    (void)printf("\t%s", p);
	}
	(void)printf("\n");
    }

    /* Print data in physical units if '-p' option selected. */
    if (pflag) {
	char *p;
	double freq = sampfreq(NULL);

	/* Print units as a second line of column headers if '-v' selected. */
	if (vflag) {
	    (void)printf("(sec)");
	    for (i = 0; i < nsig; i++) {
		p = si[sig[i]].units;
		if (p == NULL) p = "mV";
		if ((int)strlen(p) > 5) p[5] = '\0';
		/* Print the first 5 characters of each signal units string. */
		(void)printf("\t(%s)", p);
	    }
	    (void)printf("\n");
	}

	while ((to == 0L || from < to) && getvec(v) >= 0) {
	    (void)printf("%7.3lf", (double)(from++)/freq);
	    for (i = 0; i < nsig; i++)
		(void)printf("\t%7.3lf",
		    (double)(v[sig[i]] - si[sig[i]].baseline)/si[sig[i]].gain);
	    (void)printf("\n");
	}
    }

    else {
	while ((to == 0L || from < to) && getvec(v) >= 0) {
	    (void)printf("%6ld", from++);
	    for (i = 0; i < nsig; i++)
		(void)printf("\t%5d", v[sig[i]]);
	    (void)printf("\n");
	}
    }

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
 " -p          print times and samples in physical units (default: raw units)",
 " -s SIGNAL [SIGNAL ...]  print only the specified signal(s)",
 " -t TIME     stop at specified time",
 " -v          print column headings",
NULL
};

void help()
{
    int i;

    (void)fprintf(stderr, help_strings[0], pname);
    for (i = 1; help_strings[i] != NULL; i++)
	(void)fprintf(stderr, "%s\n", help_strings[i]);
}
