/* file: wfdbio.c	G. Moody	18 November 1988
			Last revised:	  29 April 1999		wfdblib 10.0.0
Low-level I/O functions for the WFDB library

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
 getwfdb		(returns the database path string)
 setwfdb		(sets the database path string)
 wfdbquiet		(suppresses WFDB library error messages)
 wfdbverbose *		(enables WFDB library error messages)
 wfdberror ***		(returns the most recent WFDB library error message)
 wfdbfile **		(returns the complete pathname of a WFDB file)

(* New in version 4.0; ** new in version 4.3; *** new in version 4.5.)

These functions, also defined here, are intended only for the use of WFDB
library functions defined elsewhere:
 wfdb_g16		(reads a 16-bit integer)
 wfdb_g32		(reads a 32-bit integer)
 wfdb_p16		(writes a 16-bit integer)
 wfdb_p32		(writes a 32-bit integer)
 wfdb_addtopath *	(adds path component of string argument to WFDB path)
 wfdb_error		(produces an error message)
 wfdb_open		(finds and opens database files)
 wfdb_checkname		(checks record and annotator names for validity)
 wfdb_setirec **	(saves current record name)

(* New in version 6.2; ** new in version 9.7)

Functions in signal.c and calib.c use the C library function strtok() to parse
lines into tokens;  this function (and its associated header file <string.h>)
may not be available in certain older C libraries (e.g., UNIX version 7 and BSD
4.2).  This file includes a portable implementation of strtok(), which can be
obtained if necessary by defining the symbol NOSTRTOK when compiling this
module.
*/

#include "wfdblib.h"
#ifdef _WINDOWS
#include <windows.h>	      /* needed for MessageBox decl (see wfdb_error) */
#endif

/* getwfdb is used to obtain a list of places in which to search for database
files to be opened for reading.  Under UNIX or MS-DOS, this list is obtained
from the shell (environment) variable WFDB, which may be set by the user
(typically as part of the login script).  A default value may be set at compile
time (DEFWFDBP in wfdblib.h);  this is necessary when compiling for the
Macintosh OS.  If the value of WFDB is of the form `@FILE', wfdb_getiwfdb()
reads the WFDB path from the specified FILE. */

static char *wfdbpath;
static int getiwfdb_count = 0;

FSTRING getwfdb()
{
    char *getenv();
    void wfdb_getiwfdb();

    if (wfdbpath == NULL) {
	wfdbpath = getenv("WFDB");
	if (wfdbpath == NULL) wfdbpath = DEFWFDBP;
    }
    getiwfdb_count = 0;
    while (*wfdbpath == '@')
	wfdb_getiwfdb();
    return (wfdbpath);
}

/* setwfdb can be called within an application to override the setting of WFDB.
For portability, as well as efficiency, it is better to use setwfdb() than
to manipulate WFDB directly (via putenv(), for example.) */

FVOID setwfdb(pt)
char *pt;
{
    wfdbpath = pt;
}

/* wfdbquiet can be used to suppress error messages from the WFDB library. */

static int error_print = 1;

FVOID wfdbquiet()
{
    error_print = 0;
}

/* wfdbverbose enables error messages from the WFDB library. */

FVOID wfdbverbose()
{
    error_print = 1;
}

static char wfdb_filename[256];

FSTRING wfdbfile(s, record)
char *s, *record;
{
    FILE *ifile;

    if ((ifile = wfdb_open(s, record, WFDB_READ))) {
	(void)fclose(ifile);
	return (wfdb_filename);
    }
    else return (NULL);
}

/* Private functions (for the use of other WFDB library functions only). */

/* The next four functions read and write integers in PDP-11 format, which is
common to both MIT and AHA database files.  The purpose is to achieve
interchangeability of binary database files between machines which may use
different byte layouts.  The routines below are machine-independent; in some
cases (notably on the PDP-11 itself), taking advantage of the native byte
layout can improve the speed.  For 16-bit integers, the low (least significant)
byte is written (read) before the high byte; 32-bit integers are represented as
two 16-bit integers, but the high 16 bits are written (read) before the low 16
bits. These functions, in common with other WFDB library functions, assume that
a byte is 8 bits, a "short" is 16 bits, an "int" is at least 16 bits, and a
"long" is at least 32 bits.  The last two assumptions are valid for ANSI C
compilers, and for almost all older C compilers as well.  If a "short" is not
16 bits, it may be necessary to rewrite wfdb_g16() to obtain proper sign
extension. */

int wfdb_g16(fp)		/* read a 16-bit integer in PDP-11 format */
FILE *fp;
{
    register int x;

    x = getc(fp);
    return ((int)((short)((getc(fp) << 8) | (x & 0xff))));
}

long wfdb_g32(fp)		/* read a 32-bit integer in PDP-11 format */
FILE *fp;
{
    long x, y;

    x = wfdb_g16(fp);
    y = wfdb_g16(fp);
    return ((x << 16) | (y & 0xffff));
}

void wfdb_p16(x, fp)		/* write a 16-bit integer in PDP-11 format */
unsigned int x;
FILE *fp;
{
    (void)putc((char)x, fp);
    (void)putc((char)(x >> 8), fp);
}

void wfdb_p32(x, fp)		/* write a 32-bit integer in PDP-11 format */
long x;
FILE *fp;
{
    wfdb_p16((unsigned int)(x >> 16), fp);
    wfdb_p16((unsigned int)x, fp);
}

/* The wfdb_error function handles error messages, normally by printing them
on the standard error output.  Its arguments can be any set of arguments which
would be legal for printf, i.e., the first one is a format string, and any
additional arguments are values to be filled into the '%*' placeholders
in the format string.  It can be silenced by invoking wfdbquiet(), or
re-enabled by invoking wfdbverbose().

There are three major versions of wfdb_error below.  The first version is
compiled by ANSI C compilers.  (A variant of this version can be used with
Microsoft Windows;  it puts the error message into a message box, rather than
using the standard error output.)  The second version is compiled by modern
UNIX C compilers (System V, Berkeley 4.x) that are not ANSI-conforming.  The
third version can be compiled by many older C compilers, if the symbol OLDC is
defined;  do so only if you are using a C library which does not include a
vsprintf function and a "stdarg.h" or "varargs.h" header file.  This third
version uses an undocumented function (_doprnt) which works for most if not all
older UNIX C compilers, and several others as well;  it is also handy for
placating BSD lint, which cannot be made to shut up about either of the first
two versions of wfdb_error (or any other functions that use `varargs').

If OLDC is not defined, the function wfdberror (without the underscore) returns
the most recent error message passed to wfdb_error (even if output was
suppressed by wfdbquiet).  This feature permits programs to handle errors
somewhat more flexibly (in windowing environments, for example, where using the
standard error output may be inappropriate).
*/

#ifndef OLDC
static char error_message[256];
#endif

FSTRING wfdberror()
{
    if (*error_message == '\0')
	sprintf(error_message,
	       "WFDB library version %d.%d.%d (%s).\n",
	       WFDB_MAJOR, WFDB_MINOR, WFDB_RELEASE, __DATE__);
    return (error_message);
}

/* First version: for ANSI C compilers and Microsoft Windows */
#if defined(__STDC__) || defined(_WINDOWS)
#include <stdarg.h>
/*VARARGS1*/
void wfdb_error(char *format, ...)
{
    va_list arguments;

    va_start(arguments, format);
#ifndef _WINDOWS		/* standard variant: use stderr output */
    (void)vsprintf(error_message, format, arguments);
    if (error_print) {
	(void)fprintf(stderr, "%s", error_message);
	(void)fflush(stderr);
    }
#else				/* MS Windows variant: use message box */
    (void)wvsprintf(error_message, format, arguments);
    if (error_print)
	MessageBox(GetFocus(), error_message, "WFDB Library Error",
		    MB_ICONASTERISK | MB_OK);
#endif
    va_end(arguments);
}

#else
# ifndef OLDC		/* Second version: for modern UNIX C compilers */
# include <varargs.h>
/*VARARGS*/
void wfdb_error(va_alist)
va_dcl
{
    va_list arguments;
    char *format;

    va_start(arguments);
#ifndef lint
    format = va_arg(arguments, char *);
#else
    format = NULL;	/* never compiled: just to keep `lint' happy */
#endif
    (void)vsprintf(error_message, format, arguments);
    if (error_print) {
	(void)fprintf(stderr, "%s", error_message);
	(void)fflush(stderr);
    }
    va_end(arguments);
}

# else			/* Third version: for many older C compilers */
/*VARARGS1*/
void wfdb_error(format, arguments)
char *format;
{
    if (error_print) {

#ifndef lint
	(void)_doprnt(format, &arguments, stderr);
#else
/* Next two lines are never compiled;  they are here just to keep lint happy */
	if (&arguments) format = NULL;
	if (format) return;
#endif
	(void)fflush(stderr);
    }
}

# endif
#endif

/* Operating system and compiler dependent code

All of the operating system and compiler dependencies in the WFDB library are
contained within the remainder of this file.

1. MS-DOS files are much more restrictively named than are UNIX files.  For
   this reason, WFDB file names are assembled in reverse order for MS-DOS
   (record name first, then file type), and the file type is truncated to the
   first three characters.  The ordering of components of the file names is
   the reason for the difference in methods of specifying files that are not
   in the current directory, as noted below in the comments for wfdb_open.  The
   function wfdb_checkname does not accept record or annotator names that would
   be illegal as parts of MS-DOS file names; this makes it less likely that
   WFDB records created under UNIX or another operating system will require
   re-naming if moved to MS-DOS.  (Note that non-MS-DOS versions of wfdb_open
   search for MS-DOS filenames as well as standard filenames.)

2. Directory separators vary:
     UNIX and variants use `/'.
     MS-DOS and OS/2 use `\'.
     Macintosh OS uses `:'.

3. Path component separators also vary:
     UNIX and variants use `:' (as in the PATH environment variable)
     MS-DOS and OS/2 use `;' (also as in the PATH environment variable;
       `:' within a path component follows a drive letter)
     Macintosh OS uses `;' (`:' is a directory separator, as noted above)

4. By default, MS-DOS files are opened in "text" mode.  Since WFDB files are
   binary, they must be opened in binary mode.  To accomplish this, ANSI C
   libraries, and those supplied with non-ANSI C compilers under MS-DOS,
   define argument strings "rb" and "wb" to be supplied to fopen();
   unfortunately, most other non-ANSI C versions of fopen do not recognize
   these as legal.  The "rb" and "wb" forms are used here for ANSI and MS-DOS
   C compilers only, and the older "r" and "w" forms are used in all other
   cases.
*/

/* For MS-DOS compilers.  Note that not all such compilers predefine the
   symbol MSDOS;  for those which do not, MSDOS is usually defined by a
   compiler command-line option in the "make" description file. */ 
#ifdef MSDOS
# define sprf(S, TYPE, RECORD)   (void)sprintf(S, "%s.%.3s", RECORD, TYPE)
# define DSEP	'\\'
# define PSEP	';'
# define RB	"rb"
# define WB	"wb"
#else

/* For other ANSI C compilers.  Such compilers must predefine __STDC__ in order
   to conform to the ANSI specification. */
# ifdef __STDC__
#  define sprf(S, TYPE, RECORD)  (void)sprintf(S, "%s.%s", TYPE, RECORD)
#  ifdef MAC	/* Macintosh only.  Be sure to define MAC in `wfdblib.h'. */
#   ifdef FIXISOCD   /* Needed if "ISO 9660 File Access" is pre-version 5.0 */
#    define sprr(S, TYPE, RECORD)  ((TYPE == (char *)NULL) ? \
				     (void)sprintf(S, "%s;1", RECORD) : \
				     (void)sprintf(S, "%s.%.3s;1",RECORD,TYPE))
#   else
#    define sprr(S, TYPE, RECORD) (void)sprintf(S, "%s.%.3s",RECORD,TYPE))
#   endif
#   define DSEP	':'
#   define PSEP	';'
#  else		/* Other ANSI C compilers (UNIX and variants). */
#  define sprr(S, TYPE, RECORD)  (void)sprintf(S, "%s.%.3s", RECORD, TYPE)
#   define DSEP	'/'
#   define PSEP	':'
#  endif
#  define RB	"rb"
#  define WB	"wb"

/* For all other C compilers, including most UNIX C compilers. */
# else
#  define sprf(S, TYPE, RECORD)  (void)sprintf(S, "%s.%s", TYPE, RECORD)
#  define sprr(S, TYPE, RECORD)  (void)sprintf(S, "%s.%.3s", RECORD, TYPE)
#  define DSEP	'/'
#  define PSEP	':'
#  define RB	"r"
#  define WB	"w"
# endif
#endif

/* wfdb_getiwfdb reads a new value for WFDB from the file named by the second
through last characters of the current value.  It maintains a use counter to
detect looping behavior;  getwfdb() resets the counter to zero. */

#ifndef SEEK_END
#define SEEK_END 2
#endif

void wfdb_getiwfdb()
{
    FILE *wfdbpfile;

    if (getiwfdb_count++ > 10) {
	wfdb_error("getwfdb: files nested too deeply\n");
	wfdbpath = "";
    }
    else if ((wfdbpfile = fopen(wfdbpath+1, RB)) == NULL)
	wfdbpath = "";
    else {
	long len;

	if (fseek(wfdbpfile, 0L, SEEK_END) == 0)
	    len = ftell(wfdbpfile);
	else len = 255;
	rewind(wfdbpfile);
	if ((wfdbpath = (char *)malloc((unsigned)len+1)) == NULL)
	    wfdbpath = "";
	else {
	    len = fread(wfdbpath, 1, (int)len, wfdbpfile);
	    while (wfdbpath[len-1] == '\n' || wfdbpath[len-1] == '\r')
		wfdbpath[--len] = '\0';
	}
	(void)fclose(wfdbpfile);
    }
}

/* wfdb_addtopath appends the path component of its string argument (i.e.
everything except the file name itself) to the end of the WFDB path, provided
that the WFDB path does not already contain this component.  This function is
invoked by readheader() (in signal.c) for each signal file name read.

The intent is to permit the use of a relatively short WFDB path, even when
using many databases.  If header files are collected into a few directories,
and absolute pathnames for the signal files are entered within them, annotation
files stored in the same directories as the signal files will also be
accessible even if those directories are not listed explicitly in the WFDB
path.  This feature may be particularly useful to MS-DOS users because of that
system's limit on the length of environment variables. */

void wfdb_addtopath(s)
char *s;
{
    char *d, *p, *t, *getenv();
    int i, j, l;
    static char *nwfdbp;

    /* Start at the end of the string and search backwards for a directory
       separator. */
    for (p = s + strlen(s) - 1; p >= s && *p != DSEP; p--)
	;

    /* A path component specifying the root directory must be treated as a
       special case;  normally the trailing directory separator is not
       included in the path component, but in this case there is nothing
       else to include. */
    if (p == s && *p == DSEP) p++;

    if (p < s) return;		/* argument did not contain a path component */

    /* If p > s, then p points to the first character following the path
       component of s. Search the current WFDB path for this path component. */
    for (d = getwfdb(), i = p-s; *d != '\0'; ) {
	/* Copy the next component of the WFDB path into wfdb_filename. */
	for (t = wfdb_filename; *d != PSEP && *d != '\0'; )
	    *t++ = *d++;
	if (t-wfdb_filename == i && strncmp(wfdb_filename, s, i) == 0)
	    return;	/* path component of s is already in WFDB path */
	if (*d != '\0') d++;	/* skip path separator */
    }

    /* If we've come this far, the path component of s was not found in the
       current WFDB path;  now append it. */
    l = strlen(wfdbpath); 	/* wfdbpath set by getwfdb() -- see above */
    if ((t = (char *)malloc((unsigned)(l + i + 2))) == NULL) {
	wfdb_error("wfdb_addtopath: insufficient memory\n");
	return;			/* WFDB path is unchanged */
    }
    (void)strcpy(t, wfdbpath);
    t[l] = PSEP;		/* append a path separator */
    for (j = 0; j < i; j++)
	t[l+j+1] = s[j];	/* append the new path component */
    t[l+i+1] = '\0';
    if (wfdbpath == nwfdbp)	/* WFDB path was previously modified by this
				   function */
	(void)free(nwfdbp);
    setwfdb(nwfdbp = t);
}

static char irec[WFDB_MAXRNL+1]; /* current record name, set by wfdb_setirec */

/* wfdb_open is used by other WFDB library functions to open a database file
for reading or writing.  wfdb_open accepts two string arguments and an integer
argument.  The first string specifies the file type ("header", "atruth", etc.),
and the second specifies the record name.  The integer argument (mode) is
either WFDB_READ or WFDB_WRITE.  Note that a function which calls wfdb_open
does not need to know the filename itself; thus all system-specific details of
file naming conventions can be hidden in wfdb_open.  If the first argument is
"-", or if the first argument is "header" and the second is "-", wfdb_open
returns a file pointer to the standard input or output as appropriate.  If
either of the string arguments is null or empty, wfdb_open takes the other as
the file name.  Otherwise, it constructs the file name by concatenating the
string arguments with a "." between them.  If the file is to be opened for
reading, wfdb_open searches for it in the list of directories obtained from
getwfdb(); output files are normally created in the current directory.  By
prefixing the file type argument (under UNIX) or the record argument (under
MS-DOS) with appropriate path specifications, files can be opened in any
directory. */

FILE *wfdb_open(s, record, mode)
char *s, *record;
int mode;
{
    char *wfdb, *p;
    FILE *ifile;

    /* Check to see if standard input or output is requested. */
    if (strcmp(s, "-") == 0 ||
	(strcmp(s, "header") == 0 && strcmp(record, "-") == 0))
	return ((mode == WFDB_READ) ? stdin : stdout);

    /* If the file is to be opened for output, use the current directory.
       An output file can be opened in another directory if the path to
       that directory is the first part of 's' (or 'record' under MSDOS). */
    if (mode == WFDB_WRITE) {
	if (record == NULL || *record == '\0') (void)strcpy(wfdb_filename, s);
	else if (s == NULL || *s == '\0') (void)strcpy(wfdb_filename, record);
	else sprf(wfdb_filename, s, record);
	return (fopen(wfdb_filename, WB));
    }

    /* If the file is to be opened for input, prepare to search the database
       directories.  The string obtained from getwfdb() is constructed in the
       same manner as PATH, with colons separating its components.  An
       empty component is interpreted as the current directory. */
    wfdb = getwfdb();
    do {
	p = wfdb_filename;
	/* Copy the next component of wfdb into wfdb_filename. */
	while (*wfdb != PSEP && *wfdb != '\0') {
	    if (*wfdb == '%') {
		/* Perform substitutions in the WFDB path where `%' is found */
		wfdb++;
		if (*wfdb == 'r') {
		    /* `%r' -> record name */
		    (void)strcpy(p, irec);
		    p += strlen(p);
		    wfdb++;
		}
		else if ('1' <= *wfdb && *wfdb <= '9' && *(wfdb+1) == 'r') {
		    /* `%Nr' -> first N characters of record name */
		    int n = *wfdb - '0';
		    int len = strlen(irec);

		    if (len < n) n = len;
		    (void)strncpy(p, irec, n);
		    p += n;
		    *p = '\0';
		    wfdb += 2;
		}
		else    /* `%X' -> X, if X is neither `r', nor a non-zero digit
			   followed by 'r' */
		    *p++ = *wfdb++;
	    }
	    else *p++ = *wfdb++;
	}
	/* Unless the WFDB component was empty, or it ended with a directory
	   separator, append a directory separator to wfdb_filename;  then
	   append the s and record components. */
	if (p != wfdb_filename) {
#ifndef MSDOS
	    if (*(p-1) != DSEP)
#else
	    if (*(p-1) != DSEP && *(p-1) != ':')
#endif
		*p++ = DSEP;
	}
	if (record == NULL || *record == '\0') (void)strcpy(p, s);
	else if (s == NULL || *s == '\0') (void)strcpy(p, record);
	else sprf(p, s, record);
	if ((ifile = fopen(wfdb_filename, RB)) != NULL) return (ifile);
#ifndef MSDOS
	/* If the previous attempt failed, and neither the record nor the s
	   component of the filename is empty, try again, swapping the
	   positions of record and s within wfdb_filename. */
	else if (record!= NULL && *record != '\0' && s != NULL && *s != '\0') {
	    sprr(p, s, record);
	    if ((ifile = fopen(wfdb_filename, RB)) != NULL) return (ifile);
	}
#endif
	/* If this file can be opened for input, return its file pointer;
	   otherwise, prepare to try the next wfdb component. */
	if (*wfdb == PSEP) wfdb++;
    } while (*wfdb != '\0');
    /* If the file was not found in any of the directories listed in wfdb,
       return a null file pointer to indicate failure. */
    return (NULL);
}

/* This function checks record and annotator names -- they must not be empty,
   and they must contain only letters, digits, underscores, and directory
   separators. */

int wfdb_checkname(p, s)
char *p, *s;
{
    do {
	if (('0' <= *p && *p <= '9') || *p == '_' || *p == DSEP ||
	    ('a' <= *p && *p <= 'z') || ('A' <= *p && *p <= 'Z'))
	    p++;
	else {
	    wfdb_error("init: illegal character %d in %s name\n", *p, s);
	    return (-1);
	}
    } while (*p);
    return (0);
}

/* This function saves the current record name (its argument) in irec (defined
above) to be substituted for `%r' in the WFDB path by wfdb_open as necessary.
wfdb_setirec is invoked by isigopen (except when isigopen is invoked
recursively to open a segment within a multi-segment record) and by annopen
(when it is about to open a file for input). */

void wfdb_setirec(p)
char *p;
{
    char *r;

    for (r = p; *r; r++)
	if (*r == DSEP) p = r+1;	/* strip off any path information */
#ifdef MSDOS
	else if (*r == ':') p = r+1;
#endif
    if (strcmp(p, "-"))	       /* don't record `-' (stdin) as record name */
	strncpy(irec, p, WFDB_MAXRNL);
}

#ifdef NOSTRTOK
char *strtok(p, sep)
char *p, *sep;
{
    char *psep;
    static char *s;

    if (p) s = p;
    if (!s || !(*s) || !sep || !(*sep)) return ((char *)NULL);
    for (psep = sep; *psep; psep++)
	if (*s == *psep) {
	    if (!(*(++s))) return ((char *)NULL);
	    psep = sep;
	}
    p = s;
    while (*s)
	for (psep = sep; *psep; psep++)
	    if (*s == *psep) {
		*s++ == '\0';
		return (p);
	    }
    return (p);
}
#endif

#ifdef _WINDLL
#ifndef _WIN32
/* Functions named LibMain and WEP are required in all 16-bit MS Windows
dynamic link libraries. The private library function wgetenv is a replacement
for the standard C getenv function, useful since the DOS environment is not
available to Windows DLLs except by use of the Windows GetDOSEnvironment
function.
*/

FINT LibMain(HINSTANCE hinst, WORD wDataSeg, WORD cbHeapSize,
	               LPSTR lpszCmdLine)
{
    if (cbHeapSize != 0)	/* if DLL data segment is moveable */
	UnlockData(0);
    return (1);
}

FINT WEP(int nParameter)
{
    if (nParameter == WEP_SYSTEM_EXIT || WEP_FREE_DLL)
	wfdbquit();	/* Always close WFDB files before shutdown or
			   when the last WFDB application exits. */
    return (1);
}


/* This is a quick-and-dirty reimplementation of getenv for the Windows 16-bit
   DLL environment.  It searches the MSDOS environment for a line beginning
   with the specified variable name, followed by `='.  This function can be
   fooled by pathologic variable names (e.g., with embedded `=' characters),
   but should be adequate for typical use. */

char FAR *wgetenv(char far *var)
{
    char FAR *ep = GetDOSEnvironment();
    int l = _fstrlen(var);
    
    while (*ep) {
	if (_fstrncmp(ep, var, l) == 0 && *(ep+l) == '=')
	    /* Got it!  Skip '=', return a pointer to the value string. */
	    return (ep+l+1);
	/* Go on to the next environment string. */
	ep += _fstrlen(ep) + 1;
    }

    /* If here, the specified variable was not found in the environment. */
    return ((char FAR *)NULL);
}

#else	/* 32-bit MS Windows DLLs require DllMain instead of LibMain and WEP */

int WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
    switch (fdwReason) {
	/* nothing to do when process (or thread) begins */
      case DLL_PROCESS_ATTACH:
      case DLL_THREAD_ATTACH:
      case DLL_THREAD_DETACH:
	break;
	/* when process terminates, always close WFDB files before shutdown or
	   when the last WFDB application exits */
      case DLL_PROCESS_DETACH:
	wfdbquit();
	break;
    }
    return (TRUE);
}
#endif
#endif

