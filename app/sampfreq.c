/* file: sampfreq.c	G. Moody	7 June 1998
			Last revised:	5 October 2001

-------------------------------------------------------------------------------
sampfreq: Print the sampling frequency of a record
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
#include <wfdb/wfdb.h>

main(argc, argv)
int argc;
char **argv;
{
    if (argc == 3 && strcmp(argv[1], "-H") == 0) {
	int nsig;
	WFDB_Siginfo *si;

	if ((nsig = isigopen(argv[2], NULL, 0)) > 0) {
	    SUALLOC(si, nsig, sizeof(WFDB_Siginfo));
	    isigopen(argv[2], si, nsig);
	    setgvmode(WFDB_HIGHRES);
	    printf("%g\n", sampfreq(NULL));
	    exit(0);
	}
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
