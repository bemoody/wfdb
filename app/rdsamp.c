/* file: rdsamp.c	G. Moody	 23 June 1983
			Last revised:   18 February 2009

-------------------------------------------------------------------------------
rdsamp: Print an arbitrary number of samples from each signal
Copyright (C) 1983-2009 George B. Moody

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

/* values for timeunits */
#define SECONDS     1
#define MINUTES     2
#define HOURS       3
#define TIMSTR      4
#define MSTIMSTR    5
#define SHORTTIMSTR 6
#define HHMMSS	    7
#define SAMPLES     8

char *pname;

main(argc, argv)
int argc;
char *argv[];
{
    char *record = NULL, *search = NULL, *prog_name();
    int highres = 0, i, isiglist, nsig, nosig = 0, pflag = 0, s, *sig = NULL,
        timeunits = SECONDS, vflag = 0;
    WFDB_Frequency freq;
    WFDB_Sample *v;
    WFDB_Siginfo *si;
    WFDB_Time from = 0L, maxl = 0L, to = 0L;
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
	  case 'P':	/* output in high-precision physical units */
	    ++pflag;	/* (fall through to case 'p') */
	  case 'p':	/* output in physical units specified */
	    ++pflag;
	    if (*(argv[i]+2) == 'd') timeunits = TIMSTR;
	    else if (*(argv[i]+2) == 'e') timeunits = HHMMSS;
	    else if (*(argv[i]+2) == 'h') timeunits = HOURS;
	    else if (*(argv[i]+2) == 'm') timeunits = MINUTES;
	    else if (*(argv[i]+2) == 'S') timeunits = SAMPLES;
	    else timeunits = SECONDS;
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
    freq = sampfreq(NULL);
    if (from > 0L && (from = strtim(argv[from])) < 0L)
	from = -from;
    if (isigsettime(from) < 0)
	exit(2);
    if (to > 0L && (to = strtim(argv[to])) < 0L)
	to = -to;
    if (nosig) {		/* print samples only from specified signals */
#ifndef lint
	if ((sig = (int *)malloc((unsigned)nosig*sizeof(int))) == NULL) {
	    (void)fprintf(stderr, "%s: insufficient memory\n", pname);
	    exit(2);
	}
#endif
	for (i = 0; i < nosig; i++) {
	    if ((s = findsig(argv[isiglist+i])) < 0) {
		(void)fprintf(stderr, "%s: can't read signal '%s'\n", pname,
			      argv[isiglist+i]);
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

    /* Print column headers if '-v' option selected. */
    if (vflag) {
	char *p, *r, *t;
	int j, l;

	if (pflag == 0) (void)printf("       sample #");
	else if (timeunits == SAMPLES) (void)printf("sample interval");
	else if (timeunits == TIMSTR || timeunits == HHMMSS) {
	    p = timstr(0L);
	    if (*p != '[')
		timeunits = HHMMSS;
	    else if (strlen(p) < 16)
		timeunits = SHORTTIMSTR;
	    if (timeunits == HHMMSS)
		printf("   Elapsed time");
	    else if (timeunits == SHORTTIMSTR)
		printf("     Time");
	    else {
		if (freq > 1.0) {
		    timeunits = MSTIMSTR;
		    (void)printf("   ");
		}
		(void)printf("   Time      Date    ");
	    }
	}
	else (void)printf("   Elapsed time");
	if ((t = malloc((strlen(record)+30) * sizeof(char))) == NULL) {
	    fprintf(stderr, "%s: insufficient memory\n", pname);
	    exit(2);
	}
	for (r = record+strlen(record); r > record; r--)
	    if (*r == '/') {
		r++;
		break;
	    }
	for (i = 0; i < nsig; i++) {
	    /* Check if a default signal description was provided.  Note that
	       if the description comes from the header file, the record name
	       may include path information that might not match that in
	       'record' (if any);  so we compare only the final part of the
	       description (p+j below) against the expected description (t). */
	    (void)sprintf(t, "%s, signal %d", r, sig[i]);
	    p = si[sig[i]].desc;
	    j = strlen(p) - strlen(t);
	    if (j > 0 && strcmp(p+j, t) == 0) {
		(void)sprintf(t, "sig %d", sig[i]);
		p = t;
	    }
	    l = strlen(p);
	    if (pflag > 1) {
		if (l > 15) p += l - 15;
		(void)printf("\t%15s", p);
	    }
	    else {
		if (l > 7) p+= l - 7;
		(void)printf("\t%7s", p);
	    }
	}
	(void)printf("\n");
    }

    /* Print data in physical units if '-p' option selected. */
    if (pflag) {
	char *p, *fmt = pflag > 1 ?  "\t%15.8lf" : "\t%7.3f";

	if (timeunits == HOURS) freq *= 3600.;
	else if (timeunits == MINUTES) freq *= 60.;

	/* Print units as a second line of column headers if '-v' selected. */
	if (vflag) {
	    char s[12];

	    switch (timeunits) {
 	    case TIMSTR:      (void)printf("(hh:mm:ss dd/mm/yyyy)"); break;
 	    case MSTIMSTR:    (void)printf("(hh:mm:ss.mmm dd/mm/yyyy)"); break;
	    case SHORTTIMSTR: (void)printf("(hh:mm:ss.mmm)"); break;
	    case HHMMSS:      (void)printf("   hh:mm:ss.mmm"); break;
	    case HOURS:       (void)printf("        (hours)"); break;
	    case MINUTES:     (void)printf("      (minutes)"); break;
	    default:
	    case SECONDS:     (void)printf("      (seconds)"); break;
	    case SAMPLES:     (void)sprintf(s, "(%g", 1./freq);
			      (void)printf("%10s sec)", s);
	    }
	    for (i = 0; i < nsig; i++) {
		char ustring[16];
	
		p = si[sig[i]].units;
		if (p == NULL) p = "mV";
		if (pflag > 1)
		    sprintf(ustring, "%14s", p);
		else
		    sprintf(ustring, "%6s", p);
		for (p = ustring+2; *p == ' '; p++)
		    ;
		*(p-1) = '(';
		(void)printf("\t%s)", ustring);
	    }
	    (void)printf("\n");
	}

	while ((to == 0L || from < to) && getvec(v) >= 0) {
	    switch (timeunits) {
	      case TIMSTR:   (void)printf("%s", timstr(-from)); break;
	      case SHORTTIMSTR:
	      case MSTIMSTR: (void)printf("%s", mstimstr(-from)); break;
	      case HHMMSS:   (void)printf("%15s", from == 0L ?
					  "0:00.000" : mstimstr(from)); break;
	      case SAMPLES:  (void)printf("%15ld", from); break;
	      default:
	      case SECONDS:  (void)printf("%15.3lf", (double)from/freq); break;
	      case MINUTES:  (void)printf("%15.5lf", (double)from/freq); break;
	      case HOURS:    (void)printf("%15.7lf", (double)from/freq); break;
	    }
	    from++;
	    for (i = 0; i < nsig; i++) {
		if (v[sig[i]] != WFDB_INVALID_SAMPLE)
		    (void)printf(fmt,
		    (double)(v[sig[i]] - si[sig[i]].baseline)/si[sig[i]].gain);
		else
		    (void)printf(pflag > 1 ? "\t%15s" : "\t%7s", "-");
	    }
	    (void)printf("\n");
	}
    }

    else {
	while ((to == 0L || from < to) && getvec(v) >= 0) {
	    (void)printf("%15ld", from++);
	    for (i = 0; i < nsig; i++)
		(void)printf("\t%7d", v[sig[i]]);
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
 " -P          same as -p, but with greater precision",
 "              -p and -P may be followed by a character to choose a time",
 "              format;  choices are:",
 "  -pd (or -Pd)  print time of day and date if known",
 "  -pe (or -Pe)  print elapsed time as <hours>:<minutes>:<seconds>",
 "  -ph (or -Ph)  print elapsed time in hours",
 "  -pm (or -Pm)  print elapsed time in minutes",
 "  -ps (or -Ps)  print elapsed time in seconds",
 "  -pS (or -PS)  print elapsed time in sample intervals",
 " -s SIGNAL [SIGNAL ...]  print only the specified signal(s)",
 " -S SIGNAL   search for a valid sample of the specified SIGNAL at or after",
 "		the time specified with -f, and begin printing then",
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
