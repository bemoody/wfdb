/* file: wrsamp.c	G. Moody	10 August 1993
			Last revised:  14 November 2002
-------------------------------------------------------------------------------
wrsamp: Select fields or columns from a file and generate a WFDB record
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

Portions of this program were derived from the `field' utility described in
"The UNIX System" by S.R. Bourne, pp. 227-9 (Addison-Wesley, 1983).
*/

#include <stdio.h>
#include <wfdb/wfdb.h>

#define isfsep(c)	((fsep && ((c)==fsep)) || ((c)==' ' || (c)=='\t'))

char *pname;

main(argc, argv)
int argc;
char *argv[];
{
    char **ap, *cp, **desc, **fp = NULL, fsep = '\0', *ifname = "(stdin)",
	*l = NULL, ofname[40], *p, *record = NULL, rsep = '\n', *prog_name();
    double freq = WFDB_DEFFREQ, gain = WFDB_DEFGAIN, scale = 1.0, v;
#ifndef atof
    double atof();
#endif
    int c, cf = 0, *fv = NULL, i, lmax = 1024, mf;
    FILE *ifile = stdin;
    long t = 0L, t0 = 0L, t1 = 0L;
#ifndef atol
    long atol();
#endif
    WFDB_Sample *vout;
    WFDB_Siginfo *si;
    unsigned int nf = 0;
    void help();

    pname = prog_name(argv[0]);

    for (i = 1; i < argc && *argv[i] == '-'; i++) {
	switch (*(argv[i]+1)) {
	  case 'c':
	    cf = -1;
	    break;
	  case 'f':
	    if (++i >= argc || (t0 = atol(argv[i])) <= 0L) {
	       (void)fprintf(stderr, "%s: starting line # must follow -f\n",
			     pname);
		exit(1);
	    }
	    break;
	  case 'F': 
	    if (++i >= argc || (freq = atof(argv[i])) <= 0.) {
	       (void)fprintf(stderr, "%s: sampling frequency must follow -F\n",
			     pname);
		exit(1);
	    }
	    break;
	  case 'G':
	    if (++i >= argc || (gain = atof(argv[i])) <= 0.) {
	       (void)fprintf(stderr, "%s: gain must follow -G\n", pname);
		exit(1);
	    }
	    break;
	  case 'h':
	    help();
	    exit(1);
	    break;
	  case 'i':
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: input file name must follow -i\n",
			      pname);
		exit(1);
	    }
	    if (ifile != stdin)
		(void)fclose(ifile);
	    if (strcmp(argv[i], "-") == 0)
		ifile = stdin;
	    else if ((ifile = fopen(argv[i], "r")) == NULL) {
		(void)fprintf(stderr, "%s: can't read `%s'\n", pname, argv[i]);
		exit(2);
	    }
	    ifname = argv[i];
	    break;
	  case 'l':
	    if (++i >= argc || (lmax = atoi(argv[i])) < 1) {
		(void)fprintf(stderr, "%s: max line length must follow -l\n",
			      pname);
		exit(1);
	    }
	    break;
	  case 'o':
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: record name must follow -o\n",
			      pname);
		exit(1);
	    }
	    record = argv[i];
	    break;
	  case 'r':
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: line separator must follow -r\n",
			      pname);
		exit(1);
	    }
	    rsep = argv[i][0];
	    break;
	  case 's':
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: field separator must follow -s\n",
			      pname);
		exit(1);
	    }
	    fsep = argv[i][0];
	    break;
	  case 't':
	    if (++i >= argc || (t1 = atol(argv[i])) <= 0L) {
	       (void)fprintf(stderr, "%s: ending line # must follow -t\n",
			      pname);
		exit(1);
	    }
	    break;
	  case 'x':
	    if (++i >= argc || (scale = atof(argv[i])) <= 0.) {
	       (void)fprintf(stderr, "%s: scaling factor must follow -x\n",
			     pname);
		exit(1);
	    }
	    break;
	  default:
	    (void)fprintf(stderr, "%s: unrecognized option %s\n", pname,
			  argv[i]);
	    exit(1);
	}
    }

    /* allocate storage for arrays */
#ifndef lint
    l = calloc(lmax, sizeof(char));
    fp = (char **)calloc(lmax, sizeof(char *));
    fv = (int *)calloc(argc - i, sizeof(int));
#endif
    if (l == NULL || fv == NULL || fp == NULL) {
	(void)fprintf(stderr, "%s: insufficient memory\n", pname);
	exit(3);
    }

    /* read arguments into fv[...] */
    while (i < argc) {
	if (sscanf(argv[i++], "%d", &fv[nf]) != 1 ||
	    fv[nf] < 0 || fv[nf] >= lmax) {
	    (void)fprintf(stderr, "%s: unrecognized argument %s\n",
			  pname, argv[--i]);
	    exit(1);
	}
	nf++;
    }

    if (nf < 1) {
	help();
	exit(1);
    }

    if ((vout = malloc(nf * sizeof(WFDB_Sample))) == NULL ||
	(si = malloc(nf * sizeof(WFDB_Siginfo))) == NULL ||
	(desc = malloc(nf * sizeof(char *))) == NULL) {
	(void)fprintf(stderr, "%s: insufficient memory\n", pname);
	exit(2);
    }

    /* open output file */
    if (record == NULL)
	(void)sprintf(ofname, "-");
    else
	(void)sprintf(ofname, "%s.dat", record);
    for (i = 0; i < nf; i++) {
	si[i].fname = ofname;
	if ((desc[i] = malloc((strlen(ifname)+20) * sizeof(char))) == NULL) {
	    (void)fprintf(stderr, "%s: insufficient memory\n", pname);
	    exit(2);
	}
	if (ifile == stdin)
	    (void)sprintf(desc[i], "column %d", fv[i]);
	else
	    (void)sprintf(desc[i], "%s, column %d", ifname, fv[i]);
	si[i].desc = desc[i];
	si[i].units = "";
	si[i].gain = gain;
	si[i].group = 0;
	si[i].fmt = 16;
	si[i].bsize = 0;
	si[i].adcres = WFDB_DEFRES;
	si[i].adczero = 0;
	si[i].baseline = 0;
    }
    if (osigfopen(si, nf) < nf || setsampfreq(freq) < 0)
	exit(2);

    /* read and copy input */
    nf--;
    cp = l;
    ap = fp;
    *ap++ = cp;
    do {
	c = getc(ifile);
	if (cp >= l + lmax) {
	    (void)fprintf(stderr,
			  "%s: line %ld truncated (> %d bytes)\n",
			  pname, t, lmax);
	    while (c != rsep && c!= EOF)
		c = getc(ifile);
	}
	if (c == rsep || c == EOF) {
	    if (cp == l && c == EOF) break;
	    *cp++ = 0;
	    mf = ap - fp;
	    if (cf) {
		if (cf < 0) cf = mf;
		else if (cf != mf)
		    (void)fprintf(stderr,
				  "%s: line %ld has incorrect field count\n",
				  pname, t);
	    }
	    if (t >= t0) {	/* write data from this line */
		for (i = 0; i <= nf; i++) {
		    if ((p = fp[fv[i]]) == NULL) {
			(void)fprintf(stderr,
				      "%s: line %ld, column %d missing\n",
				      pname, t, fv[i]);
			vout[i] = 0;
		    }
		    else if (sscanf(p, "%lf", &v) != 1) {
			(void)fprintf(stderr,
			      "%s: line %ld, column %d improperly formatted\n",
				      pname, t, fv[i]);
			vout[i] = 0;
		    }
		    else
			vout[i] = (int)(scale * v);
		}
		if (putvec(vout) < 0) break;
		for (i = 0; i <= nf; i++)
		    fp[fv[i]] = NULL;
	    }
	    if (c == EOF || (++t >= t1 && t1 > 0L)) break;
	    cp = l;
	    ap = fp;
	    *ap++ = cp;
	}
	else if (isfsep(c)) {
	    if (cp > l) {
		*cp++ = 0;
		*ap++ = cp;
	    }
	    do {
		c = getc(ifile);
	    } while (c != rsep && c != EOF && isfsep(c));
	    (void)ungetc(c, ifile);
	}
	else *cp++ = c;
    } while (c != EOF);

    if (record != NULL)
	(void)newheader(record);
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
 "usage: %s [OPTIONS ...] COLUMN [COLUMN ...]\n",
 "where COLUMN selects a field to be copied (leftmost field is column 0),",
 "and OPTIONS may include:",
 " -c          check that each input line contains the same number of fields",
 " -f N        start copying with line N (default: 0)",
 " -F FREQ     specify frequency to be written to header file (default: 250)",
 " -G GAIN     specify gain to be written to header file (default: 200)",
 " -h          print this usage summary",
 " -i FILE     read input from FILE (default: standard input)",
 " -l LEN      read up to LEN characters in each line (default: 1024)",
 " -o RECORD   save output in RECORD.dat, and generate a header file for",
 "              RECORD (default: write to standard output in format 16, do",
 "              not generate a header file)",
 " -r RSEP     interpret RSEP as the input line separator (default: \\n)",
 " -s FSEP     interpret FSEP as the input field separator (default: space",
 "              or tab)",
 " -t N        stop copying at line N (default: end of input file)",
 " -x SCALE    multiply all inputs by SCALE (default: 1)",
NULL
};

void help()
{
    int i;

    (void)fprintf(stderr, help_strings[0], pname);
    for (i = 1; help_strings[i] != NULL; i++)
	(void)fprintf(stderr, "%s\n", help_strings[i]);
}
