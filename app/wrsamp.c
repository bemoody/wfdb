/* file: wrsamp.c	G. Moody	10 August 1993
			Last revised:    1 March 2010

-------------------------------------------------------------------------------
wrsamp: Select fields or columns from a file and generate a WFDB record
Copyright (C) 1993-2010 George B. Moody

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
#include <stdlib.h>
#include <wfdb/wfdb.h>

/* The following definition yields dither with a triangular PDF in (-1,1). */
#define DITHER	        (((double)rand() + (double)rand())/RAND_MAX - 1.0)

char *pname;

char *read_line(FILE *ifile, char rsep)
{
    static char *buf;
    int c;
    static size_t i = 0, length = 0;

    while ((c = getc(ifile)) != rsep) {
	if (i >= length) {
	    if (length == 0) length = 512;
	    length *= 2;
	    SREALLOC(buf, length+2, sizeof(char));
	}
 	if (c == EOF) {
	    SFREE(buf);
	    return (buf = NULL);
	}
	buf[i++] = c;
    }
    buf[i] = '\0';
    i = 0;
    return (buf);
}

int line_has_alpha(char *line)
{
    char *p;

    for (p = line; *p; p++)
    	if (('A' <= *p && *p <= 'Z') || ('a' <= *p && *p <= 'z'))
	    return (1);
    return (0);
}

int line_has_tab(char *line)
{
    char *p;

    for (p = line; *p; p++)
    	if (*p == '\t')
	    return (1);
    return (0);
}

struct parsemode {
    char *delim;	/* characters that delimit tokens */
    char collapse;	/* if non-zero, collapse consecutive delimiters */
    char esc;		/* if non-zero, next character is literal unless null */
    char *quotepair[];	/* pairs of characters that open and close tokens */
};

struct tokenarray {
    int ntokens;	/* number of tokens */
    int maxtokens;	/* number of allocated token pointers */
    char *token[];	/* token pointers */
};

typedef struct parsemode Parsemode;
typedef struct tokenarray Tokenarray;

/* The function parseline() parses its first argument, a null-terminated string
('line'), into a Tokenarray ('ta').  On return, the token pointers (ta->token[])
address null-terminated tokens copied from 'line'; 'line' is not altered.

The function returns the number of tokens found in ta->ntokens, and the
token array on return contains either ta->ntokens or ta->maxtokens valid
elements, whichever is smaller.  If ta->ntokens > ta->maxtokens, the token
array contains pointers to the first ta->maxtokens tokens.

The second argument of parseline, pmode, is a pointer to a Parsemode
(also defined above).  The caller can set up pmode to specify which characters
in 'line' are token delimiters, which pairs of characters can be used to quote
a token that may have embedded delimiters, whether to treat consecutive
delimiters as if they surround an empty token or as a single delimiter, and
which character acts as an escape (to cause the next character to be treated as
a literal character in a token, rather than as a delimiter, quote, or escape
character).

If pmode is NULL, parseline behaves as if called with pmode = defpmode.  If
pmode->delim is NULL, parseline sets pmode->delim = defpmode.delim.
*/

Parsemode defpmode = {
  " \t\r\n,", /* delimiter characters can be space, tab, CR, LF, or comma */
  1,	/* collapse consecutive delimiters */
  '\\',	/* treat any character following a backslash as a literal */
  { "''", "\"\"", "()", "[]", "{}", "<>", NULL } /* quote characters */
};

static char *tbuf;
static Tokenarray *ta;

Tokenarray *parseline(char *line, Parsemode *pmode)
{
    int i, m = (strlen(line) + 1)/2, n = 0, state = 0;
    char d, *p, *q = NULL;

    SSTRCPY(tbuf, line);
    p = tbuf-1;
    SREALLOC(ta, sizeof(int)*2 + sizeof(char *)*m, 1);
    ta->maxtokens = m;

    if (pmode == NULL)
	pmode = &defpmode;
    else if (pmode->delim == NULL)
	pmode->delim = defpmode.delim;

    while (*(++p)) {	/* for each character in the line */

	/* is *p an escape character? */
	if (pmode->esc && *p == pmode->esc) {
	    if (*(p+1) == '\0')
		break;
	    if (state == 0) { /* start a new token */
		state = 1;
		ta->token[n++] = p;
	    }
	    p++;	/* include the next character in the token */
	    continue;
	}

	/* is *p the character needed to complete a quoted string? */
	if (q) {
	    if (*p == *q) { *p = '\0'; q = NULL; }
	    continue;
	}

	/* is *p a delimiter character? */
	i = 0;
        while (d = pmode->delim[i++]) {
	    if (*p == d) {
		*p = '\0';	/* replace delimiter with null */
		if (state == 0) { /* not in a token */
		    if (pmode->collapse == 0)
			ta->token[n++] = p;	/* count an empty token */
		}
		state = 0;
		break;
	    }
	}

	/* is *p an open-quote character? */
	i = 0;
	while (q = pmode->quotepair[i++]) {  /* q is an open-quote character */
	    if (*p == *q) { /* *p is first character of a quoted string */
		if (state == 0) { /* start a new token */
		    ta->token[n++] = p+1;
		    state = 1;
		}
		q++; /* *q is now the matching close-quote character */
		break;
	    }
	}

	if (d == '\0' && q == NULL) { 	/* p must be part of a token */
	    if (state == 0) {
		ta->token[n++] = p;	/* start a new token */
		state = 1;
	    }
	}
    }

    ta->ntokens = n;
    return (ta);
}

main(argc, argv)
int argc;
char *argv[];
{
    char **ap, *cp, **desc, *gain = "", *ifname = "(stdin)",
	*line = NULL, ofname[40], *p, *record = NULL, rsep = '\n',
	*scale = "", sflag = 0, trim = 0, **units, *prog_name();
    static char btime[25];
    double freq = WFDB_DEFFREQ, *scalef, v;
#ifndef atof
    double atof();
#endif
    int c, cf = 0, dflag = 0, format = 16, *fv = NULL, i, labels, mf, zflag = 0;
    FILE *ifile = stdin;
    long t = 0L, t0 = 0L, t1 = 0L;
#ifndef atol
    long atol();
#endif
    Tokenarray *ta;
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
	  case 'd':
	    dflag = 1;
	    srand((long)0);	/* seed for dither RNG -- replace 0 with a
				   different value to get different dither */
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
	    if (++i >= argc) {
	       (void)fprintf(stderr, "%s: gain(s) must follow -G\n", pname);
		exit(1);
	    }
	    gain = argv[i];
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
	    if (++i >= argc) --i;
	    (void)fprintf(stderr, "%s: -l is obsolete, ignored\n", pname);
	    break;
	  case 'o':
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: record name must follow -o\n",
			      pname);
		exit(1);
	    }
	    record = argv[i];
	    break;
	  case 'O':
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: output format must follow -O\n",
			      pname);
		exit(1);
	    }
	    format = atoi(argv[i]);
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
		(void)fprintf(stderr, "%s: field separator(s) must follow -s\n",
			      pname);
		exit(1);
	    }
	    sflag = 1;
	    defpmode.delim = argv[i];
	    break;
	  case 't':
	    if (++i >= argc || (t1 = atol(argv[i])) <= 0L) {
	       (void)fprintf(stderr, "%s: ending line # must follow -t\n",
			      pname);
		exit(1);
	    }
	    break;
	  case 'x':
	    if (++i >= argc) {
	       (void)fprintf(stderr, "%s: scaling factor(s) must follow -x\n",
			     pname);
		exit(1);
	    }
	    scale = argv[i];
	    break;
          case 'z':	 /* ignore column 0 */
	    zflag = 1;
	    break;
	  default:
	    (void)fprintf(stderr, "%s: unrecognized option %s\n", pname,
			  argv[i]);
	    exit(1);
	}
    }

    /* read the first line of the input file */
    if ((line = read_line(ifile, rsep)) == NULL) {
	if (rsep != '\n') {
	    if (record == NULL)
		(void)fprintf(stderr,
			      "%s: use -o to specify the output record name",
			      pname);
	    else
		(void)fprintf(stderr,
		  "%s: no record separators in input\n"
		  "Try specifying a different separator after the -r option.\n",
			      pname);
	}
	else
	    (void)fprintf(stderr, "%s: no newlines in input\n", pname);
	exit(3);
    }
 
    /* unless -s was given, note if it contains any tab characters */
    if (sflag == 0 && line_has_tab(line))
	defpmode.delim = "\t";

    /* note if it contains any alphabetic characters */
    labels = line_has_alpha(line);

    /* parse it into tokens */
    ta = parseline(line, NULL);

    /* read selected column numbers into fv[...] */
    if (i < argc) {
	SUALLOC(fv, argc - i, sizeof(int));
	while (i < argc) {
	    if (sscanf(argv[i++], "%d", &fv[nf]) != 1 ||
		fv[nf] < 0 || fv[nf] >= ta->ntokens) {
		(void)fprintf(stderr, "%s: unrecognized argument %s\n",
			      pname, argv[--i]);
		exit(1);
	    }
	    nf++;
	}
    }
    /* if no columns were specified, copy all columns (or all except 0) */
    else {
	int i, j;

	nf = ta->ntokens - zflag;
	SUALLOC(fv, nf, sizeof(int));
	for (i = 0, j = zflag; j <= nf; i++, j++)
	    fv[i] = j;
    }

    /* allocate arrays */
    SUALLOC(vout, nf, sizeof(WFDB_Sample));
    SUALLOC(si, nf, sizeof(WFDB_Siginfo));
    SUALLOC(scalef, nf, sizeof(double));

     /* open the output record */
    if (record == NULL)
	(void)sprintf(ofname, "-");
    else
	(void)sprintf(ofname, "%s.dat", record);
    for (i = 0; i < nf; i++) {
	si[i].fname = ofname;
	si[i].desc = NULL;
	if (labels) { /* set the signal descriptions from the column headings */
	    char *p = ta->token[fv[i]], *q;

	    while (*p == ' ') p++;
	    q = p + strlen(p);
	    while (*(q-1) == ' ') q--;
	    *q = '\0';
	    SSTRCPY(si[i].desc, p);
	}
	else {
	    char tdesc[16];

	    (void)sprintf(tdesc, "column %d", fv[i]);
	    SSTRCPY(si[i].desc, tdesc);
	}
	si[i].units = "";
	si[i].group = 0;
	si[i].fmt = format;
	si[i].spf = 1;
	si[i].bsize = 0;
	switch (format) {
	  case 80:  si[i].adcres = 8; break;
	  case 310:
	  case 311: si[i].adcres = 10; break;
	  case 212: si[i].adcres = 12; break;
	  case 24:  si[i].adcres = 24; break;
	  case 32:  si[i].adcres = 32; break;
	  default:  si[i].adcres = WFDB_DEFRES;
	}
	si[i].adczero = 0;
	si[i].baseline = 0;
	while (*gain == ' ')
	    gain++;
	if (sscanf(gain, "%lf", &(si[i].gain)) < 1)
	    si[i].gain = (i == 0) ? WFDB_DEFGAIN : si[i-1].gain;
	else
	    while (*gain != '\0' && *gain != ' ')
		gain++;
	while (*scale == ' ')
	    scale++;
	if (sscanf(scale, "%lf", &(scalef[i])) < 1)
	    scalef[i] = (i == 0) ? 1.0 : scalef[i-1];
	else
	    while (*scale != '\0' && *scale != ' ')
		scale++;
    }

    if (labels) {	/* read the second line of input */
	if ((line = read_line(ifile, rsep)) == NULL) {
	    (void)fprintf(stderr, "%s: no input data\n", pname);
	    exit(4);
	}
	if (line_has_alpha(line)) {
	    ta = parseline(line, NULL);
	    /* Copy units strings after trimming any surrounding spaces,
	       parentheses, or brackets, and after replacing embedded spaces
	       with underscores. */
	    for (i = 0; i < nf; i++) {
		char *p = ta->token[fv[i]], *q;

		while (*p == ' ') p++;
		if (*p == '(' || *p == '[') p++;
		q = p + strlen(p);
		while (*(q-1) == ' ') q--;
		if (*(q-1) == ')' || *(q-1) == ']') q--;
		*q = '\0';
		while (--q > p)
		    if (*q == ' ') *q = '_';
		si[i].units = NULL;
		SSTRCPY(si[i].units, p);
	    }
	    line = read_line(ifile, rsep);
	}
    }

    /* discard any additional lines containing text */
    while (line_has_alpha(line))
	line = read_line(ifile, rsep);

    if (osigfopen(si, nf) < nf || setsampfreq(freq) < 0)
	exit(2);

    /* skip any unwanted samples at the beginning */
    for (t = 0; t < t0; t++)
	line = read_line(ifile, rsep);

    /* pick up base time if it's there */
    if (zflag) {
	if (*line == '[') {
	    strncpy(btime, line+1, 23);
	    setbasetime(btime);
	}
	else if (*(line+1) == '[') {
	    strncpy(btime, line+2, 23);
	    setbasetime(btime);
	}
    }

    /* read and copy samples */
    while (line != NULL && (t1 == 0L || t++ < t1)) {
	ta = parseline(line, NULL);
	for (i = 0; i < nf; i++) {
	    double v;

	    if (sscanf(ta->token[fv[i]], "%lf", &v) == 1) {
		//	    if (strcmp(ta->token[fv[i]], "-")) {
		v *= scalef[i];
		if (dflag) v+= DITHER;
		if (v >= 0) vout[i] = (WFDB_Sample)(v + 0.5);
		else vout[i] = (WFDB_Sample)(v - 0.5);
	    }
	    else
		vout[i] = WFDB_INVALID_SAMPLE;
	}
	if (putvec(vout) < 0) break;
	line = read_line(ifile, rsep);
    }

    /* write the header */
    (void)setsampfreq(freq);
    if (record != NULL)
	(void)newheader(record);
    wfdbquit();
    SFREE(fv);
    SFREE(scalef);
    SFREE(si);
    SFREE(ta);
    SFREE(tbuf);
    SFREE(vout);
    exit(0);
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
 "usage: %s [OPTIONS ...] [COLUMN ...]\n",
 "where COLUMN selects a field to be copied (leftmost field is column 0),",
 "and OPTIONS may include:",
 " -c          check that each input line contains the same number of fields",
 " -d          add dither to the input",
 " -f N        start copying with line N (default: 0)",
 " -F FREQ     specify frequency to be written to header file (default: 250)",
 " -G GAIN     specify gain(s) to be written to header file (default: 200)",
 " -h          print this usage summary",
 " -i FILE     read input from FILE (default: standard input)",
 " -o RECORD   save output in RECORD.dat, and generate a header file for",
 "              RECORD (default: write to standard output in format 16, do",
 "              not generate a header file)",
 " -O FORMAT   save output in the specified FORMAT (default: 16)",
 " -r RSEP     interpret RSEP as the input line separator (default: linefeed)",
 " -s FSEP     interpret any character in FSEP as an input field separator",
 "             (default: space, tab, carriage-return, or comma)  ",
 " -t N        stop copying at line N (default: end of input file)",
 " -x SCALE    multiply inputs by SCALE factor(s) (default: 1)",
 " -z          don't copy column 0 unless explicitly specified",
 "",
 "The input is a text file with a sample of each signal on each line. Samples",
 "can be separated by tabs, spaces, commas, or any combination of these.",
 "Consecutive separators are equivalent to single separators.",
 "",  
 "To specify different GAIN or SCALE values for each output signal, provide",
 "a quoted list of values, e.g., -G \"100 50\" defines the gain for signal 0",
 "as 100, and the gain for signal 1 (and any additional signals) as 50.",
 "",
 "If the first input line contains any alphabetic characters, it is assumed",
 "to contain column headings, which are copied as signal descriptions into",
 "RECORD.hea.  If the second input line also contains alphabetic characters,",
 "it is assumed to specify the physical units for each column, which are",
 "copied as units strings into RECORD.hea.  The first line containing samples",
 "is line 0.",
 "",
 "If no COLUMN is specified, all fields are copied in order; column 0 can be",
 "omitted in this case by using the '-z' option.",
NULL
};

void help()
{
    int i;

    (void)fprintf(stderr, help_strings[0], pname);
    for (i = 1; help_strings[i] != NULL; i++) {
	(void)fprintf(stderr, "%s\n", help_strings[i]);
	if (i % 23 == 0) {
	    char b[5];
	    (void)fprintf(stderr, "--More--");
	    (void)fgets(b, 5, stdin);
	    (void)fprintf(stderr, "\033[A\033[2K");  /* erase "--More--";
						      assumes ANSI terminal */
	}
    }
}
