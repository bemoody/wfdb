/* file: snip.c		G. Moody	30 July 1989
			Last revised:  7 February 2003
-------------------------------------------------------------------------------
snip: Copy an excerpt of a database record
Copyright (C) 2003 George B. Moody

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
char *argv[];
{
    char *info, *irec = NULL, *nrec = NULL, *ofname, *orec, *startp = "0:0",
         *xinfo, *prog_name();
    int a, i, nann = 0, nsig;
    long from = 0L, to = 0L, nsamp;
    WFDB_Anninfo *ai;
    WFDB_Annotation annot;
    WFDB_Sample *v;
    WFDB_Siginfo *si;
    void help();

    pname = prog_name(argv[0]);

    for (i = 1; i < argc; i++) {
	if (*argv[i] == '-') switch (*(argv[i]+1)) {
	  case 'a':	/* annotator(s) follow */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: annotator(s) must follow -a\n",
			      pname);
		exit(1);
	    }
	    /* Accept the next argument unconditionally as an annotator name;
	       accept additional arguments until we find one beginning with
	       `-', or the end of the argument list. */
	    a = i;
	    do {
	        nann++;
	    } while (++i < argc && *argv[i] != '-');
	    if (i < argc) i--;
	    break;
	  case 'f':	/* starting time */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: time must follow -f\n", pname);
		exit(1);
	    }
	    startp = argv[i];
	    break;
	  case 'h':	/* help requested */
	    help();
	    exit(0);
	    break;
	  case 'i':	/* input record name */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: input record name must follow -i\n",
			      pname);
		exit(1);
	    }
	    irec = argv[i];
	    break;
	  case 'n':	/* new record name */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: new record name must follow -n\n",
			      pname);
		exit(1);
	    }
	    nrec = argv[i];
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
    if (irec == NULL || nrec == NULL) {
	help();
	exit(1);
    }

    /* Verify that the output record can be written. */
    if (newheader(nrec) < 0) exit(2);

    /* Determine the number of signals. */
    if ((nsig = isigopen(irec, NULL, 0)) < 0) exit(2);

    /* Evaluate the time limits. */
    if ((from = strtim(startp)) < 0L)
	from = -from;
    if (to > 0L) {
	if ((to = strtim(argv[to])) < 0L)
	    to = -to;
    }

    /* Copy the signals, if any. */
    if (nsig > 0) {
	/* Allocate data structures for nsig signals. */
        if ((v = malloc(nsig * sizeof(WFDB_Sample))) == NULL ||
	    (si = malloc(nsig * sizeof(WFDB_Siginfo))) == NULL ||
	    (ofname = malloc((strlen(nrec)+5) * sizeof(char))) == NULL) {
	    (void)fprintf(stderr, "%s: insufficient memory\n", pname);
	    exit(2);
	}

	/* Open the input signals. */
	if (isigopen(irec, si, (unsigned)nsig) != nsig) exit(2);

	/* Open the output signals. */
	(void)sprintf(ofname, "%s.dat", nrec);
	for (i = 0; i < nsig; i++) {
	    si[i].fname = ofname;
	    si[i].group = 0;
	    if (i > 0) si[i].fmt = si[0].fmt;
	}
	if (osigfopen(si, (unsigned)nsig) != nsig) exit(2);

	/* Copy the selected segment. */
	if (isigsettime(from) < 0) exit(2);
	nsamp = (to == 0L) ? -1L : to - from;
	while ((nsamp == -1L || nsamp-- > 0L) &&
	    getvec(v) == nsig && putvec(v) == nsig)
	    ;
	free(ofname);
	free(si);
	free(v);
    }

    /* Copy the annotations, if any. */
    if (nann > 0) {
        /* Allocate data structures for nann annotators. */
	if ((ai = malloc(nann * sizeof(ai))) == NULL ||
	    (orec = malloc((strlen(nrec)+2) * sizeof(char))) == NULL) {
	    (void)fprintf(stderr, "%s: insufficient memory\n", pname);
	    exit(2);
	}
	for (i = 0; i < nann; i++) {
	    ai[i].name = argv[a++];
	    ai[i].stat = WFDB_READ;
	}

	/* Open the input annotators. */
	if (annopen(irec, ai, nann) < 0) exit(2);

	/* Open the output annotators. */
	(void)sprintf(orec, "+%s", nrec);
	for (i = 0; i < nann; i++)
	    ai[i].stat = WFDB_WRITE;
	if (annopen(orec, ai, nann) < 0) exit(2);

        /* Copy the selected segment. */
        for (i = 0; i < nann; i++) {
	    while (getann((unsigned)i, &annot) == 0) {
	        if (annot.time >= from && (to == 0L || annot.time < to)) {
		    annot.time -= from;
		    if (putann((unsigned)i, &annot) < 0) break;
		}
	    }
	}
	free(orec);
	free(ai);
    }

    /* Clean up. */
    (void)newheader(nrec);

    /* Copy info strings from the input header file into the new one.
       Suppress error messages from the WFDB library. */
    wfdbquiet();
    if (info = getinfo(irec))
        do {
	    (void)putinfo(info);
	} while (info = getinfo((char *)NULL));

    /* Append additional info summarizing what snip has done. */
    if (xinfo =
	malloc((strlen(pname)+strlen(irec)+strlen(startp)+50)* sizeof(char))) {
        (void)sprintf(xinfo, "Produced by %s from record %s, beginning at %s",
		      pname, irec, startp);
	(void)putinfo(xinfo);
	free(xinfo);
    }
    wfdbverbose();
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
 "usage: %s -i IREC -n NREC [OPTIONS ...]\n",
 "where IREC is the name of the record to be converted, NREC is the name to",
 "be given to the record to be generated, and OPTIONS may include any of:",
 " -a ANNOTATOR [ANNOTATOR ...]  copy annotations for the specified ANNOTATOR",
 "              from IREC;  two or more ANNOTATORs may follow -a",
 " -f TIME     begin at specified time",
 " -h          print this usage summary",
 " -t TIME     stop at specified time",
 "To change the sampling frequency, gain, or storage format, to select a",
 "subset of signals, or to re-order signals, use `xform'.",
NULL
};

void help()
{
    int i;

    (void)fprintf(stderr, help_strings[0], pname);
    for (i = 1; help_strings[i] != NULL; i++)
	(void)fprintf(stderr, "%s\n", help_strings[i]);
}
