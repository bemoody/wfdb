/* file: wfdbinit.c	G. Moody	23 May 1983
			Last revised:	29 April 1999		dblib 10.0.0
WFDB library functions wfdbinit, wfdbquit, and wfdbflush
_______________________________________________________________________________
wfdb: a library for reading and writing annotated waveforms (time series data)
Copyright (C) 1999 George B. Moody

This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option) any
later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU Library General Public License for more
details.

You should have received a copy of the GNU Library General Public License along
with this library; if not, write to the Free Software Foundation, Inc., 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA.

You may contact the author by e-mail (george@mit.edu) or postal mail
(MIT Room E25-505A, Cambridge, MA 02139 USA).  For updates to this software,
please visit the author's web site (http://ecg.mit.edu/).
_______________________________________________________________________________

This file contains definitions of the following WFDB library functions:
 wfdbinit	(opens annotation files and input signals)
 wfdbquit	(closes all annotation and signal files)
 wfdbflush	(writes all buffered output annotation and signal files)
*/

#include "wfdblib.h"

FINT wfdbinit(record, aiarray, nann, siarray, nsig)
char *record;			/* record name */
WFDB_Anninfo *aiarray;		/* annotation file information array */
WFDB_Siginfo *siarray;		/* signal information array */
unsigned int nann, nsig;	/* number of entries in afarray and siarray */
{
    int stat;

    if ((stat = annopen(record, aiarray, nann)) == 0)
	stat = isigopen(record, siarray, (int)nsig);
    return (stat);
}

FVOID wfdbquit()
{
    wfdb_anclose();	/* close annotation files, reset variables */
    wfdb_sigclose();	/* close signals, reset variables */
}

FVOID wfdbflush()	/* write all buffered output to files */
{
    wfdb_oaflush();	/* flush buffered output annotations */
    wfdb_osflush();	/* flush buffered output samples */
}
