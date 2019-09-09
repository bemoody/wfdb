/* file: wfdbinit.c	G. Moody	 23 May 1983
			Last revised:   28 April 2020   	wfdblib 10.7.0
WFDB library functions wfdbinit, wfdbquit, and wfdbflush
_______________________________________________________________________________
wfdb: a library for reading and writing annotated waveforms (time series data)
Copyright (C) 1983-2012 George B. Moody

This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option) any
later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU Library General Public License for more
details.

You should have received a copy of the GNU Library General Public License along
with this library; if not, see <http://www.gnu.org/licenses/>.

You may contact the author by e-mail (wfdb@physionet.org) or postal mail
(MIT Room E25-505A, Cambridge, MA 02139 USA).  For updates to this software,
please visit PhysioNet (http://www.physionet.org/).
_______________________________________________________________________________

This file contains definitions of the following WFDB library functions:
 wfdbinit	(opens annotation files and input signals)
 wfdbquit	(closes all annotation and signal files)
 wfdbflush	(writes all buffered output annotation and signal files)
*/

#include "wfdblib.h"

FINT wfdbinit(char *record, const WFDB_Anninfo *aiarray, unsigned int nann,
	      WFDB_Siginfo *siarray, unsigned int nsig)
{
    int stat;

    if ((stat = annopen(record, aiarray, nann)) == 0)
	stat = isigopen(record, siarray, (int)nsig);
    return (stat);
}

FVOID wfdbquit(void)
{
    wfdb_anclose();	/* close annotation files, reset variables */
    wfdb_oinfoclose();	/* close info file */
    wfdb_sigclose();	/* close signals, reset variables */
    resetwfdb();	/* restore the WFDB path */
    wfdb_sampquit();	/* release sample data buffer */
    wfdb_freeinfo();	/* release info strings */
}

FVOID wfdbflush(void)	/* write all buffered output to files */
{
    wfdb_oaflush();	/* flush buffered output annotations */
    wfdb_osflush();	/* flush buffered output samples */
}
