/* file ihr.c		G. Moody      12 November 1992
			Last revised:  9 November 1999

-------------------------------------------------------------------------------
ihr: Generate instantaneous heart rate data from annotation file
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
please visit the author's web site (http://ecg.mit.edu/).
_______________________________________________________________________________

*/

#include <stdio.h>
#ifndef __STDC__
extern void exit();
#endif

#include <wfdb/wfdb.h>
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
    double ihr, ihrlast, mhr = 70., sps, tol = 10.0, atof(), fabs();
    int i, j, lastann = NOTQRS, lastint = 1, xflag = 0;
    long from = 0L, to = 0L, lasttime;
    static WFDB_Anninfo ai;
    WFDB_Annotation annot;
    void help();

    pname = prog_name(argv[0]);

    /* Interpret command-line options. */
    for (i = 1; i < argc; i++) {
	if (*argv[i] == '-') switch (*(argv[i]+1)) {
	  case 'a':	/* annotator follows */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: annotator must follow -a\n",
			      pname);
		exit(1);
	    }
	    ai.name = argv[i];
	    break;
	  case 'd':	/* tolerance (max HR change beat to beat, in bpm) */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: tolerance must follow -d\n",
			      pname);
		exit(1);
	    }
	    tol = atof(argv[i]);
	    break;
	  case 'f':	/* starting time follows */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: starting time must follow -f\n",
			      pname);
		exit(1);
	    }
	    from = i;		/* to be converted to sample intervals below */
	    break;
	  case 'h':	/* print usage summary and quit */
	    help();
	    exit(0);
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
	  case 'x':	/* exclude intervals adjacent to abnormal beats */
	    xflag = 1;
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

    ai.stat = WFDB_READ;
    if (annopen(record, &ai, 1) < 0) /* open annotation file */
	exit(2);

    if (from && iannsettime(strtim(argv[(int)from])) < 0) exit(2);
    if (to) to = strtim(argv[(int)to]);

    while (getann(0, &annot) == 0 && (to == 0L || annot.time <= to)) {
	if (!isqrs(annot.anntyp)) continue;
	if (map1(annot.anntyp) == NORMAL) {
	    ihr = sps*60./(annot.time - lasttime);
	    mhr += (ihr - mhr)/10.;
	    if (lastann == NORMAL &&
		fabs(ihr - ihrlast) < tol &&
		fabs(ihr - mhr) < tol) {
		if (xflag) {
		    if (lastint == 0)
			(void)printf("%g %g\n", lasttime/sps, ihr);
		}
		else
		    (void)printf("%g %g %d\n", lasttime/sps, ihr, lastint);
		lastint = 0;
	    }
	    else
		lastint = 1;
	    lastann = NORMAL;
	    lasttime = annot.time;
	    ihrlast = ihr;
	}
	else {
	    lastint = 1;
	    lastann = NOTQRS;
	}
    }
    exit(0);			/*NOTREACHED*/
}

char *prog_name(s)
char *s;
{
    char *p = s + strlen(s);

#ifdef MSDOS
    while (p >= s && *p != '\\' && *p != ':') {
	if (*p == '.')
	    *p = '\0';			/* strip off extension */
	if ('A' <= *p && *p <= 'Z')
	    *p += 'a' - 'A';		/* convert to lower case */
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
    " -d TOL   reject beat-to-beat HR changes > TOL bpm (default: TOL = 10)",
    " -f TIME  start at specified TIME",
    " -h       print this usage summary",
    " -t TIME  stop at specified TIME",
    " -x       exclude intervals adjacent to abnormal beats",
    NULL
};

void help()
{
    int i;

    (void)fprintf(stderr, help_strings[0], pname);
    for (i = 1; help_strings[i] != NULL; i++)
	(void)fprintf(stderr, "%s\n", help_strings[i]);
}
