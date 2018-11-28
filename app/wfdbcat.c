/* file: wfdbcat.c	G. Moody	3 March 2000
			Last revised:	18 May 2018
-------------------------------------------------------------------------------
wfdbcat: Find and concatenate WFDB files on the standard output
Copyright (C) 2000 George B. Moody

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, see <http://www.gnu.org/licenses/>.

You may contact the author by e-mail (wfdb@physionet.org) or postal mail
(MIT Room E25-505A, Cambridge, MA 02139 USA).  For updates to this software,
please visit PhysioNet (http://www.physionet.org/).
_______________________________________________________________________________

*/

#include <stdio.h>
#include <wfdb/wfdb.h>
#include <wfdb/wfdblib.h>

#ifndef __STDC__
extern void exit();
#endif

#ifndef BUFSIZE
#define BUFSIZE 1024	/* bytes read at a time */
#endif

main(argc, argv)
int argc;
char *argv[];
{
    char *pname, *prog_name();
    int i = 0;
    size_t n;
    static char buf[BUFSIZE];
    WFDB_FILE *ifile;

    pname = prog_name(argv[0]);
    if (argc < 2 || (argc == 2 && strcmp(argv[1], "-h") == 0)) {
	(void)fprintf(stderr, "usage: %s FILE [FILE ...]\n", pname);
	exit(1);
    }

    for (i = 1; i < argc; i++) {
	if ((ifile = wfdb_open(argv[i], NULL, WFDB_READ)) == NULL)
	    (void)fprintf(stderr, "%s: `%s' not found\n", pname, argv[i]);
	else {
	    while ((n = wfdb_fread(buf, 1, sizeof(buf), ifile)) &&
		   (n == fwrite(buf, 1, n, stdout)))
	    	;
	    wfdb_fclose(ifile);
	}
    }

    exit(0);
    /*NOTREACHED*/
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
