/* file: snip.c		G. Moody	30 July 1989
			Last revised:	10 September 2001
-------------------------------------------------------------------------------
snip: Copy an excerpt of a database record
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
#ifndef __STDC__
extern void exit();
#endif
#include <wfdb/wfdb.h>

char *pname;

main(argc, argv)
int argc;
char *argv[];
{
    char *irec = NULL, *nrec = NULL, *prog_name();
    int i, j, nsig, v[WFDB_MAXSIG];
    long from = 0L, to = 0L, nsamp;
    unsigned int nann = 0;
    static char desc[WFDB_MAXSIG][40], ofname[32], orec[32];
    static WFDB_Siginfo si[WFDB_MAXSIG];
    static WFDB_Anninfo ai[WFDB_MAXANN];
    static WFDB_Annotation annot[WFDB_MAXANN];
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

    if ((nsig = isigopen(irec, si, WFDB_MAXSIG)) < 0) exit(2);
    if (nann != 0 && annopen(irec, ai, nann) < 0) exit(2);
    if (newheader(nrec) < 0) exit(2);
    (void)sprintf(ofname, "%s.dat", nrec);
    for (i = 0; i < nsig; i++) {
	si[i].fname = ofname;
	si[i].group = 0;
	if (i > 0) si[i].fmt = si[0].fmt;
	(void)sprintf(desc[i], "record %s, signal %d", irec, i);
	if (si[i].desc == NULL || strcmp(si[i].desc, desc[i]) == 0) {
	    (void)sprintf(desc[i], "record %s, signal %d", nrec, i);
	    si[i].desc = desc[i];
	}
    }
    if (osigfopen(si, (unsigned)nsig) < nsig) exit(2);
    (void)sprintf(orec, "+%s", nrec);
    if (from > 0L) {
	if ((from = strtim(argv[from])) < 0L)
	from = -from;
	if (isigsettime(from) < 0)
	    exit(2);
    }
    if (to > 0L) {
	if ((to = strtim(argv[to])) < 0L)
	    to = -to;
    }
    nsamp = (to == 0L) ? -1L : to - from;
    for (j = 0; j < nann; j++)
	ai[j].stat = WFDB_WRITE;
    if (nann != 0 && annopen(orec, ai, nann) < 0) exit(2);

    /* Copy selected segment of signals. */
    while ((nsamp == -1L || nsamp-- > 0L) &&
	   getvec(v) == nsig && putvec(v) == nsig)
	;

    /* Copy selected segment of annotations. */
    for (i = 0; i < nann; i++) {
	if (getann((unsigned)i, &annot[i]) == 0) {
	    do {
		annot[i].time -= from;
		if (annot[i].time >= 0L && putann((unsigned)i, &annot[i]) < 0)
		    break;
	    } while (getann((unsigned)i, &annot[i]) == 0 &&
		     (to == 0L || annot[i].time < to));
	}
    }

    /* Clean up. */
    (void)newheader(nrec);
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
