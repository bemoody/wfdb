/* file: db.h		G. Moody	13 June 1983
			Last revised:   29 April 1999	wfdblib 10.0.0
Compatibility definitions for (WF)DB library

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
please visit PhysioNet (http://www.physionet.org/).
_______________________________________________________________________________

The DB library has been renamed (in version 10.0.0 and later versions) as the
WFDB library.  New applications should be written to use the definitions
contained in <wfdb/wfdb.h>.  Existing applications written for use with the
DB library may be compiled without change in almost all cases using this header
file and linking them with the WFDB library.  Note that the WFDB library uses
the environment variables WFDB and WFDBCAL, rather than DB and DBCAL, so that
DB applications that manipulate these variables directly (rather than via
getdb/setdb) may need revision to accommodate these changes.
*/

#ifndef db_DB_H		/* avoid multiple definitions */
#define db_DB_H

#include <wfdb/wfdb.h>

/* DB library version */
#define DB_MAJOR     WFDB_MAJOR
#define DB_MINOR     WFDB_MINOR
#define DB_RELEASE   WFDB_RELEASE

/* Simple data types */
#define DB_Sample    WFDB_Sample
#define DB_Time      WFDB_Time
#define DB_Date      WFDB_Date
#define DB_Frequency WFDB_Frequency
#define DB_Gain      WFDB_Gain
#define DB_Group     WFDB_Group
#define DB_Signal    WFDB_Signal
#define DB_Annotator WFDB_Annotator

/* Array sizes */
#define DB_MAXANN    WFDB_MAXANN
#define	DB_MAXSIG    WFDB_MAXSIG
#define DB_MAXSPF    WFDB_MAXSPF
#define DB_MAXRNL    WFDB_MAXRNL
#define DB_MAXUSL    WFDB_MAXUSL
#define DB_MAXDSL    WFDB_MAXDSL

/* DB_anninfo '.stat' values */
#define READ         WFDB_READ
#define WRITE        WFDB_WRITE
#define AHA_READ     WFDB_AHA_READ
#define AHA_WRITE    WFDB_AHA_WRITE

/* DB_siginfo '.fmt' values
#define FMT_LIST     WFDB_FMT_LIST
#define NFMTS        WFDB_NFMTS

/* Default signal specifications */
#define DEFFREQ	     WFDB_DEFFREQ
#define DEFGAIN	     WFDB_DEFGAIN
#define DEFRES	     WFDB_DEFRES

/* getvec operating modes */
#define DB_LOWRES    WFDB_LOWRES
#define DB_HIGHRES   WFDB_HIGHRES

/* calinfo '.caltype' values
#define AC_COUPLED   WFDB_AC_COUPLED
#define DC_COUPLED   WFDB_DC_COUPLED
#define CAL_SQUARE   WFDB_CAL_SQUARE
#define CAL_SINE     WFDB_CAL_SINE
#define CAL_SAWTOOTH WFDB_CAL_SAWTOOTH
#define CAL_UNDEF    WFDB_CAL_UNDEF

/* Structure definitions */
#define DB_siginfo   WFDB_siginfo
#define DB_calinfo   WFDB_calinfo
#define DB_anninfo   WFDB_anninfo
#define DB_ann       WFDB_ann

/* Composite data types */
#define DB_Siginfo   WFDB_Siginfo
#define DB_Calinfo   WFDB_Calinfo
#define DB_Anninfo   WFDB_Anninfo
#define DB_Annotation WFDB_Annotation

/* Renamed library functions (others not renamed) */
#define dbinit	     wfdbinit
#define dbgetskew    wfdbgetskew
#define dbsetskew    wfdbsetskew
#define dbsetstart   wfdbsetstart
#define dbquit       wfdbquit
#define dbquiet      wfdbquiet
#define dbverbose    wfdbverbose
#define dberror      wfdberror
#define setdb        setwfdb
#define getdb        getwfdb
#define dbfile       wfdbfile
#define dbflush      wfdbflush
#define db_error     wfdb_error

#endif
