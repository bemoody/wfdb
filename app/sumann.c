/* file: sumann.c	G. Moody       5 February 1982
			Last revised:  3 October 2002

-------------------------------------------------------------------------------
sumann: Tabulates annotations
Copyright (C) 2002 George B. Moody

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
#ifndef BSD
# include <string.h>
#else
# include <strings.h>
#endif
#ifdef __STDC__
# include <stdlib.h>
#else
extern void exit();
# ifndef NOMALLOC_H
#  include <malloc.h>
# else
extern char *malloc();
# endif
#endif

#include <wfdb/wfdb.h>
#include <wfdb/ecgcodes.h>
#include <wfdb/ecgmap.h>

#define MAXR	100

static char *rstring[MAXR+1] = { "",
	"N",	"SVTA",	"VT",	"AFIB", "AFL",	"B",	"T",	"IVR",	"AB",
	"BII",	"NOD",	"PREX",	"SBR",	"VFL",	"P", 	NULL,	NULL,	NULL,
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL
};

static char *nstring[6] = {
	"",	"Unreadable\t",	"Both signals clean",
	"Signal 0 noisy",	"Signal 1 noisy",
	"Both signals noisy" };

char *pname;

main(argc, argv)
int argc;
char *argv[];
{
    static WFDB_Anninfo ai;
    WFDB_Annotation annot;
    int i, rhythm = 0, noise = 2, qflag = 0;
    static long tab[64], rtab[MAXR+1], ntab[6];
    static long rtime[MAXR+1], ntime[6], r0, n0, from_time, to_time;
    char *record = NULL, *prog_name();
    void help();

    pname = prog_name(argv[0]);

    /* Accept old syntax. */
    if (argc >= 3 && argv[1][0] != '-') {
	ai.name = argv[1];
	record = argv[2];
	i = 3;
    }
    else
	i = 1;

    /* Interpret command-line options. */
    for ( ; i < argc; i++) {
	if (*argv[i] == '-') switch (*(argv[i]+1)) {
	  case 'a':	/* annotator follows */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: annotator must follow -a\n",
			      pname);
		exit(1);
	    }
	    ai.name = argv[i];
	    break;
	  case 'f':	/* starting time follows */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: starting time must follow -f\n",
			      pname);
		exit(1);
	    }
	    from_time = i;   /* to be converted to sample intervals below */
	    break;
	  case 'h':	/* print usage summary and quit */
	    help();
	    exit(0);
	    break;
	  case 'q':       /* list only QRS annotations in event table */
	    qflag = 1;
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
	    to_time = i;
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

    if (sampfreq(record) < 0.)
	(void)setsampfreq(WFDB_DEFFREQ);

    ai.stat = WFDB_READ;
    if (annopen(record, &ai, 1) < 0)	/* open annotation file */
	exit(2);

    rtime[0] = strtim("0.5");
    for (i = 1; i <= MAXR; i++)
	rtime[i] = rtime[0];
    for (i = 1; i < 6; i++)
	ntime[i] = rtime[0];

    if (from_time)
	from_time = strtim(argv[(int)from_time]);
    if (from_time < 0L) from_time = -from_time;

    if (to_time)
	to_time = strtim(argv[(int)to_time]);
    else
	to_time = strtim("e");
    if (to_time < 0L) to_time = -to_time;

    if (to_time > 0L && to_time < from_time) {
	long tt;

	tt = from_time; from_time = to_time; to_time = tt;
    }

    while (getann(0, &annot) >= 0 && (to_time == 0L || annot.time < to_time)) {
	if (annot.time < from_time) {
	    if (annot.anntyp == RHYTHM) {
		r0 = from_time;
		for (i = 1, rhythm = 0; i <= MAXR; i++)
		    if (annot.aux == NULL) break;
		    else if (rstring[i] == NULL) {
			if (rstring[i] =
			    (char *)malloc((unsigned)(*annot.aux - 1))) {
			    (void)strcpy(rstring[i], annot.aux+2);
			    rhythm = i;
			}
			break;
		    }
		    else if (strcmp(annot.aux+2, rstring[i]) == 0) {
			rhythm = i;
			break;
		    }
	    }
	    else if (annot.anntyp == NOISE) {
		n0 = from_time;
		if ((noise = annot.subtyp + 2) > 5 || noise < 1)
		    noise = 0;
	    }
	    continue;
	}
	tab[annot.anntyp]++;
	if (annot.anntyp == RHYTHM) {
	    if (rhythm) {
		rtab[rhythm]++;
		rtime[rhythm] += annot.time - r0;
	    }
	    r0 = annot.time;
	    for (i = 1, rhythm = 0; i <= MAXR; i++)
		if (annot.aux == NULL) break;
	        else if (rstring[i] == NULL) {
		    if (rstring[i] = (char *)malloc((unsigned)(*annot.aux-1))){
			(void)strcpy(rstring[i], annot.aux+2);
			rhythm = i;
		    }
		    break;
		}
		else if (strcmp(annot.aux+2, rstring[i]) == 0) {
		    rhythm = i;
		    break;
		}
	}
	else if (annot.anntyp == NOISE) {
	    if (noise) {
		ntab[noise]++;
		ntime[noise] += annot.time - n0;
	    }
	    n0 = annot.time;
	    if ((noise = annot.subtyp + 2) > 5 || noise < 1)
		noise = 0;
	}
    }			

    if (to_time == 0L) to_time = annot.time;
    if (rhythm) {
	rtab[rhythm]++;
	rtime[rhythm] += to_time - r0;
    }
    if (noise) {
	ntab[noise]++;
	ntime[noise] += to_time - n0;
    }
    for (i = 0; i < 64; i++)
      if (tab[i] != 0L && (qflag == 0 || isqrs(i)))
	    (void)printf("%s\t%6ld\n", annstr(i), tab[i]);
    (void)printf("\n");
    for (i = 1; i <= MAXR; i++)
	if (rtab[i] != 0L)
	    (void)printf("%s\t%6ld episode%c %s\n",
			rstring[i], rtab[i],
			rtab[i] == 1L ? ' ' : 's', timstr(rtime[i]));
    (void)printf("\n");
    for (i = 1; i <= 5; i++)
	if (ntab[i] != 0L)
	    (void)printf("%s\t%6ld episode%c %s\n",
			nstring[i], ntab[i],
			ntab[i] == 1L ? ' ' : 's', timstr(ntime[i]));
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
 " -f TIME  start at specified TIME",
 " -h       print this usage summary",
 " -q       list only QRS annotations in the event table",
 " -t TIME  stop at specified TIME",
NULL
};

void help()
{
    int i;

    (void)fprintf(stderr, help_strings[0], pname);
    for (i = 1; help_strings[i] != NULL; i++)
	(void)fprintf(stderr, "%s\n", help_strings[i]);
}
