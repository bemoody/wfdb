/* file: wvscript.c	G. Moody	21 May 1997
			Last revised:	30 April 1999

-------------------------------------------------------------------------------
wvscript: start wview from a wavescript-compatible script
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

This program can be installed as a helper application for a web browser such
as Netscape, to handle application/x-wavescript (.xws) files under Windows.
(Under Linux or UNIX, use wavescript for this purpose.)
*/

#include <stdio.h>

main(argc, argv)
int argc;
char **argv;
{
    FILE *script;
    static char record[20], annot[20] = "-", pstart[40];
    static char script_line[128], command_line[128];
    
    if (argc < 2) {
        fprintf(stderr, "usage: wvscript script-file\n");
        exit(1);
    }
    if ((script = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "wvscript: can't open %s\n", argv[1]);
        exit(2);
    }
    while (fgets(script_line, sizeof(script_line), script)) {
        char *p, *q;

        if (strncmp(script_line, "-r", 2) == 0) {
            for (p = script_line+3, q = record; *p != '+' && *p != '\n'; )
                *q++ = *p++;
            *q = '\0';
        }
        else if (strncmp(script_line, "-a", 2) == 0) {
            for (p = script_line+3, q = annot; *p != '\n'; )
                *q++ = *p++;
            *q = '\0';
        }
        else if (strncmp(script_line, "-f", 2) == 0) {
            for (p = script_line+3, q = pstart; *p != '\n'; )
                *q++ = *p++;
            *q = '\0';
        }
    }
    sprintf(command_line, "wview %s %s %s\n", record, annot, pstart);
    system(command_line);
    return (0);
}

