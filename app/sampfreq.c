/* file: sampfreq.c	G. Moody	7 June 1998
			Last revised:	3 May 1999

-------------------------------------------------------------------------------
sampfreq: Print the sampling frequency of a record
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
#include <wfdb/wfdb.h>

main(argc, argv)
int argc;
char **argv;
{
    if (argc == 3 && strcmp(argv[1], "-H") == 0) {
	static WFDB_Siginfo s[WFDB_MAXSIG];

	setgvmode(WFDB_HIGHRES);
	isigopen(argv[2], s, WFDB_MAXSIG);
	printf("%g\n", sampfreq(NULL));
	exit(0);
    }
    else if (argc == 2) {
       printf("%g\n", sampfreq(argv[1]));
       exit(0);
    }

    fprintf(stderr, "usage: %s [-H] RECORD\n", argv[0]);
    fprintf(stderr, " where RECORD is the name of the input record\n");
    fprintf(stderr, " (use -H to get the maximum sampling frequency of a multifrequency record)\n");
    exit(1);
}
