/* file: wfdblib.h	G. Moody	13 April 1989
                        Last revised:   29 April 1999         wfdblib 10.0.0
External definitions for WFDB library private functions

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

These definitions may be included in WFDB library source files to permit them
to share information about library private functions, which are not intended
to be invoked directly by user programs.  By convention, all externally defined
symbols reserved to the library begin with the characters "wfdb_".
*/

/* Define the symbol _WINDLL if this library is to be compiled as a Microsoft
Windows DLL.  Note that a DLL's private functions (such as those listed below)
*cannot* be called by Windows user programs, which can call only DLL functions
that use the Pascal calling interface (see wfdb.h).  To compile this library
for use in a Microsoft Windows application, but *not* as a DLL, define the
symbol _WINDOWS instead of _WINDLL. */
/* #define _WINDLL */

#if defined(_WINDLL)
#if !defined(_WINDOWS)
#define _WINDOWS
#endif
#define WFDBNOSORT
#endif

#if defined(_WINDOWS) && !defined(MSDOS)
#define MSDOS
#endif

/* Define the symbol MSDOS if this library is to be used under MS-DOS or MS
Windows.  Note: MSDOS is predefined by some MS-DOS and MS Windows compilers. */
/* #define MSDOS */

/* Uncomment the next line if this software is to be compiled on a Macintosh.*/
/* #define MAC */
/* Macintosh users only:  If the version of "ISO 9660 File Access" in the
"System:Extensions" folder is earlier than 5.0, either update your system
software (recommended) or uncomment the next line. */
/* #define FIXISOCD */

/* DEFWFDBP is the default value of the WFDB path if the WFDB environment
variable is not set.  In most cases, it is sufficient to use an empty string
for this purpose (thus restricting the search for DB files to the current
directory).  DEFWFDBP must not be NULL, however.  The default given below for
the Macintosh specifies that the WFDB path is to be read from the file
udb/dbpath.mac on the third edition of the MIT-BIH Arrhythmia Database CD-ROM
(which has a volume name of `MITADB3'); you may prefer to use a file on a
writable disk for this purpose to make reconfiguration possible.  See getwfdb()
in wfdbio.c for further information. */
#ifdef MAC
# ifdef FIXISOCD
#  define DEFWFDBP	"@MITADB3:UDB:DBPATH.MAC;1"
# else
#  define DEFWFDBP	"@MITADB3:UDB:DBPATH.MAC"
# define __STDC__
# endif
#else
# define DEFWFDBP	""
#endif

/* DEFWFDBC is the name of the default WFDB calibration file, used if the
WFDBCAL environment variable is not set.  This name need not include path
information if the calibration file is located in a directory included in the
WFDB path.  The value given below is the name of the standard calibration file
supplied on the various CD-ROM databases.  DEFWFDBC may be NULL if you prefer
not to have a default calibration file.  See calopen() in calib.c for further
information. */
#define DEFWFDBC "dbcal"

#ifndef FILE
#include <stdio.h>
/* stdin/stdout may not be defined in some environments (e.g., for MS Windows
   DLLs).  Defining them as NULL here allows the WFDB library to be compiled in
   such environments (it does not allow use of stdin/stdout when the operating
   environment does not provide them, however). */
#ifndef stdin
#define stdin NULL
#endif
#ifndef stdout
#define stdout NULL
#endif
#endif

#include "wfdb.h"

/* The following block is needed only to declare the values returned by
malloc() (either char * or void *) and sprintf() (either int or char *).
The object code should be correct even if these declarations are not. */
#if defined(__STDC__) || defined(_WINDOWS)
# include <stdlib.h>
#else
# ifndef NOMALLOC_H
# include <malloc.h>
# else
extern char *malloc();
# endif
# ifdef ISPRINTF
extern int sprintf();
# else
#  ifndef MSDOS
extern char *sprintf();
#  endif
# endif
#endif

/* Declare the values returned by atof() and atol() (not needed for ANSI C
or Microsoft Windows C compilers, which have these declarations in stdlib.h,
included above). */
#if !defined(__STDC__) && !defined(_WINDOWS)
extern double atof();
extern long atol();
#endif

#ifndef BSD
# include <string.h>
#else		/* for Berkeley UNIX only */
# include <strings.h>
# define strchr index
#endif

#ifdef _WINDOWS
#ifndef _WIN32	/* these definitions are needed for 16-bit MS Windows only */
#define strcat _fstrcat
#define strchr _fstrchr
#define strcmp _fstrcmp
#define strcpy _fstrcpy
#define strlen _fstrlen
#define strncmp _fstrncmp
#define strtok _fstrtok
#endif
#endif

/* Define function prototypes for ANSI C, MS Windows C, and C++ compilers */
#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus) || defined(_WINDOWS)
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* These functions are defined in wfdbio.c */
extern FILE *wfdb_open(char *file_type, char *record, int mode);
extern int wfdb_checkname(char *name, char *description);
extern int wfdb_g16(FILE *fp);
extern long wfdb_g32(FILE *fp);
extern void wfdb_p16(unsigned int x, FILE *fp);
extern void wfdb_p32(long x, FILE *fp);
extern void wfdb_addtopath(char *pathname);
extern void wfdb_error(char *format_string, ...);
extern void wfdb_setirec(char *record_name);

/* These functions are defined in signal.c */
extern void wfdb_sigclose(void);
extern void wfdb_osflush(void);

/* These functions are defined in annot.c */
extern void wfdb_anclose(void);
extern void wfdb_oaflush(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#else        /* declare only function return types for non-ANSI C compilers */

extern FILE *wfdb_open();
extern int wfdb_checkname(), wfdb_g16();
extern long wfdb_g32();
extern void wfdb_p16(), wfdb_p32(), wfdb_addtopath(), wfdb_error(),
    wfdb_setirec(), wfdb_sigclose(), wfdb_anclose(), wfdb_osflush(),
    wfdb_oaflush();

/* Some non-ANSI C libraries (e.g., version 7, BSD 4.2) lack an implementation
of strtok();  define NOSTRTOK to compile the portable version in wfdbio.c. */
# ifdef NOSTRTOK
extern char *strtok();
# endif

#endif
