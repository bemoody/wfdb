/* file: ann2rr.c		G. Moody	 16 May 1995
				Last revised:  19 February 2001
-------------------------------------------------------------------------------
ann2rr: Calculate RR intervals from an annotation file
Copyright (C) 2000 George B. Moody

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
#ifndef __STDC__
extern void exit();
#endif

#include <wfdb/wfdb.h>
#define map1
#define map2
#define ammap
#define mamap
#define annpos
#include <wfdb/ecgmap.h>

char *pname;

main(argc, argv)	
int argc;
char *argv[];
{
    char *record = NULL, *prog_name();
    double sps, spm, sph, rrsec;
    int cflag=0, i, j, pflag=0, previous_annot_valid=0, vflag=0, xflag=0;
    long beat_number = 0L, from = 0L, to = 0L, rr, tp = 0L, atol();
    static char flag[ACMAX+1];
    static WFDB_Anninfo ai;
    static WFDB_Annotation annot;
    void help();

    pname = prog_name(argv[0]);

    /* Interpret command-line options. */
    for (i = flag[0] = 1; i < argc; i++) {
	if (*argv[i] == '-') switch (*(argv[i]+1)) {
	  case 'a':	/* annotator follows */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: annotator must follow -a\n",
			      pname);
		exit(1);
	    }
	    ai.name = argv[i];
	    break;
	  case 'c':    	/* print intervals between consecutive valid
			   annotations only */
	    cflag = 1;
	    break;  
	  case 'f':	/* starting time follows */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: starting time must follow -f\n",
			      pname);
		exit(1);
	    }
	    from = i;   /* to be converted to sample intervals below */
	    break;
	  case 'h':	/* print usage summary and quit */
	    help();
	    exit(0);
	    break;
	  case 'p':	/* annotation mnemonic(s) follow */
	    if (++i >= argc || !isann(j = strann(argv[i]))) {
		(void)fprintf(stderr,
			      "%s: annotation mnemonic(s) must follow -p\n",
			      pname);
		exit(1);
	    }
	    flag[j] = 1;
	    /* The code above not only checks that there is a mnemonic where
	       there should be one, but also allows for the possibility that
	       there might be a (user-defined) mnemonic beginning with `-'.
	       The following lines pick up any other mnemonics, but assume
	       that arguments beginning with `-' are options, not mnemonics. */
	    while (++i < argc && argv[i][0] != '-')
		if (isann(j = strann(argv[i]))) flag[j] = 1;
	    if (i == argc || argv[i][0] == '-') i--;
	    flag[0] = 0;
	    break;
	  case 'r':	/* input record name follows */
	    if (++i >= argc) {
		(void)fprintf(stderr,
			      "%s: input record name must follow -r\n",
			      pname);
		exit(1);
	    }
	    record = argv[i];
	    break;
	  case 't':	/* ending time follows */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: end time must follow -t\n", pname);
		exit(1);
	    }
	    to = i;
	    break;
	  case 'v':	/* verbose mode: include time as well as RR */
	    vflag = 1;
	    break;
	  case 'x':	/* use alternate time format */
	    switch (*(argv[i]+2)) {
	      case 'h': xflag = 3; break;
	      case 'm': xflag = 2; break;
	      case 's':
	      default:  xflag = 1; break;
	    }
	    break;
	  default:
	    (void)fprintf(stderr, "%s: unrecognized option %s\n",
			  pname, argv[i]);
	    exit(1);
	}
	else {
	    (void)fprintf(stderr, "%s: unrecognized argument %s\n",
			  pname, argv[i]);
	    exit(1);
	}
    }
    if (record == NULL || ai.name == NULL) {
	help();
	exit(1);
    }

    if ((sps = sampfreq(record)) < 0.)
	(void)setsampfreq(sps = WFDB_DEFFREQ);
    spm = 60.0*sps;
    sph = 60.0*spm;

    ai.stat = WFDB_READ;
    if (annopen(record, &ai, 1) < 0)	/* open annotation file */
	exit(2);

    if (from) {
	if (*argv[(int)from] == '#') {
	    if ((beat_number = atol(argv[(int)from]+1)) < 0L) beat_number = 0L;
	    while (beat_number > 0L && getann(0, &annot) == 0)
		if (isqrs(annot.anntyp)) beat_number--;
	    if (beat_number > 0L) exit(2);
	}
	else if (iannsettime(strtim(argv[(int)from])) < 0) exit(2);
    }
    if (to) {
	if (*argv[(int)to] == '#') {
	    if ((beat_number = atol(argv[(int)to]+1)) <  1L) beat_number = 1L;
	    to = 0L;
	}
	else {
	    beat_number = -1L;
	    to = strtim(argv[(int)to]);
	}
    }

    tp = from;
    while (getann(0, &annot) == 0 && (to == 0L || annot.time <= to)) {
	if (!isann(annot.anntyp)) continue;
	if ((flag[0] && isqrs(annot.anntyp)) || flag[annot.anntyp]) {
	    if (cflag == 0 || previous_annot_valid == 1) {
		rr = annot.time - tp;
		if (vflag) {	/* print elapsed time */
		  switch (xflag) {
		    default:
		    case 0: (void)printf("%ld\t", annot.time); break;
		    case 1: (void)printf("%.3lf\t", annot.time/sps); break;
		    case 2: (void)printf("%.3lf\t", annot.time/spm); break;
		    case 3: (void)printf("%.3lf\t", annot.time/sph); break;
		  }
		}	
		/* print RR interval */
		if (xflag) (void)printf("%.3lf\n", rr/sps);
		else (void)printf("%ld\n", rr);
	    }
	    tp = annot.time;
	    previous_annot_valid = 1;
	}
	else if (cflag)
	    previous_annot_valid = 0;
	if (beat_number > 0L && isqrs(annot.anntyp) && --beat_number == 0L)
	    break;
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
 "usage: %s -r RECORD -a ANNOTATOR [OPTIONS ...]\n",
 "where RECORD and ANNOTATOR specify the input, and OPTIONS may include:",
 " -c       print intervals between consecutive valid annotations only",
 " -f TIME  start at specified TIME",
 " -h       print this usage summary",
 " -p TYPE [TYPE ...]  print intervals between annotations of specified TYPEs",
 "                      only",
 " -t TIME  stop at specified TIME",
 " -v       print elapsed times as well as RR intervals",
 " -x       use alternate format (times and RR intervals in seconds)",
 " -xh      use alternate format (times in hours, RR intervals in seconds)",
 " -xm      use alternate format (times in minutes, RR intervals in seconds)",
 " -xs      same as -x",
NULL
};

void help()
{
    int i;

    (void)fprintf(stderr, help_strings[0], pname);
    for (i = 1; help_strings[i] != NULL; i++)
	(void)fprintf(stderr, "%s\n", help_strings[i]);
}
