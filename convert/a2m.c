/* file: a2m.c		G. Moody	 9 June 1983
			Last revised:  13 February 2001

-------------------------------------------------------------------------------
a2m: Convert an AHA format annotation file to MIT format
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

This program converts four types of AHA format annotation files to MIT format:

 Type	Description

   0	AHA-format annotation files produced by WFDB library-based
	programs that use AHA_WRITE mode and `putann' (the `m2a'
	translator is an example of such a program).  Time is
	measured in sample intervals from the beginning of the record,
	and MIT annotation codes are included and read into the `chan'
	field of the annotation by `getann';  if such codes are
	present (i.e., if `chan' is non-zero), the AHA code is
	ignored.

   1	Standard AHA annotation files from short-format AHA DB
	distribution tapes.  Time is measured in milliseconds from
	the beginning of the test period (five minutes, or 75000 sample
	intervals at 4 ms/sample interval, after the first sample).

   2	Standard AHA annotation files from long-format AHA DB
	distribution tapes.  Time is measured as for filetype 1,
	except that the test period begins 2.5 hours (2,225,000 sample
	intervals) after the first sample.

   3	`Compressed' (*.ano) annotation files from AHA DB floppy
	diskettes.  Time is measured as for files of type 1.
*/

#include <stdio.h>
#ifdef __STDC__
#define RB	"rb"
#else
#ifdef MSDOS
#define RB	"rb"
#else
#define RB	"r"
extern void exit();
#endif
#endif

#include <wfdb/wfdb.h>
#define isqrs
#define map1
#define map2
#define mamap
#define annpos
#include <wfdb/ecgmap.h>

char *pname;

main(argc, argv)
int argc;
char *argv[];
{
    char *ifname = NULL, *oann = NULL, *record = NULL, *prog_name();
    int i, type = 0, getcann();
    long offset = 0L;
    unsigned int nann = 2;
    WFDB_Anninfo afarray[2];
    static WFDB_Annotation annot;
    void help();

    /* Read and interpret command-line arguments. */
    pname = prog_name(argv[0]);
    for (i = 1; i < argc; i++) {
	if (*argv[i] == '-') switch (*(argv[i]+1)) {
	  case 'a':	/* annotator name follows */
	    if (++i >= argc) {
		(void)fprintf(stderr,
			      "%s: output annotator name must follow -a\n",
			      pname);
		exit(1);
	    }
	    oann = argv[i];
	    break;
	  case 'h':	/* help requested */
	    help();
	    exit(1);
	    break;
	  case 'i':	/* input file name follows */
	    if (++i >= argc) {
		(void)fprintf(stderr,
		      "%s: name of AHA-format input file must follow -i\n",
			      pname);
		exit(1);
	    }
	    ifname = argv[i];
	    break;
	  case 'r':	/* record name follows */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: record name must follow -a\n",
			      pname);
		exit(1);
	    }
	    record = argv[i];
	    break;
	  case 's':	/* time shift follows */
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: time shift must follow -s\n",
			      pname);
		exit(1);
	    }
	    offset = i;	/* save position in arg list, convert to time
			   later when sampling frequency is known */
	    break;
	  case 't':	/* file type follows */
	    if (++i >= argc || (type = atoi(argv[i])) < 0 || type > 3) {
		(void)fprintf(stderr, "%s: file type (0-3) must follow -t\n");
		exit(1);
	    }
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

    /* Quit if input filename, annotator, or record name was not specified. */
    if (ifname == NULL || oann == NULL || record == NULL) {
	help();
	exit(1);
    }
		 
    /* Attach the input file to the standard input. */
    if (strcmp(ifname, "-") != 0 && freopen(ifname, RB, stdin) == NULL) {
	(void)fprintf(stderr, "%s: can't read input file `%s'\n", pname,
		      ifname);
	exit(2);
    }

    /* Open the annotation files. */
    afarray[0].name = oann;
    afarray[0].stat = WFDB_WRITE;
    if (type != 3) {
	afarray[1].name = "-";
	afarray[1].stat = WFDB_AHA_READ;
    }
    else
	nann = 1;
    if (annopen(record, afarray, nann) < 0)
	exit(2);

    /* Set the time shift. */
    if (sampfreq(record) == 0.)
	(void)setsampfreq(250.);
    if (offset) offset = strtim(argv[offset]);
    else {
	switch (type) {
	  case 0:
	    break;
	  case 1:
	  case 3:
	    offset = strtim("5:0");
	    break;
	  case 2:
	    offset = strtim("2:30:0");
	    break;
	}
    }

    /* Reformat the annotations. */
    switch (type) {
      case 0:
	while (getann(0, &annot) >= 0) {
	    if (annot.chan) {	/* MIT annotation code present */
		annot.anntyp = annot.chan;
		annot.chan = 0;
	    }
	    annot.time += offset;
	    if (putann(0, &annot) < 0)
		break;
	}
	break;
      case 1:
      case 2:
	while (getann(0, &annot) >= 0) {
	    annot.time = (annot.time >> 2) + offset;	/* 4 msec/sample */
	    (void)putann(0, &annot);
	}
	break;
      case 3:
	while (getcann(&annot) >= 0) {
	    annot.time = (annot.time >> 2) + offset;	/* 4 msec/sample */
	    (void)putann(0, &annot);
	}
	break;
    }

    wfdbquit();
    exit(0);	/*NOTREACHED*/
}

int getcann(ap)
WFDB_Annotation *ap;
{
    char buf[6];
    long h, m, l;

    if (fread(buf, 1, 6, stdin) != 6) {		/* hard EOF */
	(void)fprintf(stderr, "unexpected EOF in input file\n");
	return (-1);
    }
    /* Compressed AHA format:
       buf[0]	AHA label (`N', `V', `F', `R', `E', `P', `Q', `[', `]', or `U')
       buf[1]	high byte of ahatime, or 0xff if at soft EOF
       buf[2]	middle byte of ahatime
       buf[3]	low byte of ahatime
       buf[4]	high byte of annotation serial number (ignored here)
       buf[5]	low byte of serial number
     */
    
    if ((unsigned)(buf[1] & 0xff) == 0xff)	/* soft EOF */
	return (-1);

    ap->anntyp = ammap(buf[0]);	     /* convert AHA label to annotation code */

    if (buf[0] == 'U')		     /* AHA `U' (unreadable data) label */
	ap->subtyp = -1;		/* decode as NOISE, subtype = -1 */
    else
	ap->subtyp = 0;			/* all others have subtype = 0 */

    h = (long)buf[1] & 0xff;
    m = (long)buf[2] & 0xff;
    l = (long)buf[3] & 0xff;
    ap->time = (h << 16) | (m << 8) | l; /* time in milliseconds from the
					   start of the annotated segment */
    return (0);
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
    "usage: %s -i AHAFILE -r RECORD -a ANNOTATOR [ OPTIONS ... ]\n",
    "where AHAFILE is the name of the AHA-format input annotation file,",
    "RECORD is the record name, ANNOTATOR is the output annotator,",
    "and OPTIONS may include:",
    " -h         print this usage summary",
    " -s TIME    shift annotations by TIME",
    "             (defaults: 0 for TYPE 0,",
    "                      5:0 for TYPE 1 or 3,",
    "                   2:30:0 for TYPE 2)",
    " -t TYPE    specify type of file to be converted:",
    "             0 for files produced by `putann',",
    "             1 for short-format tape files,",
    "             2 for long-format tape files, or",
    "             3 for compressed files from diskettes",
    NULL
};

void help()
{
    int i;

    (void)fprintf(stderr, help_strings[0], pname);
    for (i = 1; help_strings[i] != NULL; i++)
	(void)fprintf(stderr, "%s\n", help_strings[i]);
}
