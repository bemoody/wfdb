/* file: wfdbf.c	G. Moody	 23 August 1995
			Last revised:     29 April 2020 	wfdblib 10.7.0

_______________________________________________________________________________
wfdbf: Fortran wrappers for the WFDB library functions
Copyright (C) 1995-2013 George B. Moody

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

General notes on using the wrapper functions

Wrappers are provided below for all of the WFDB library functions except
setmsheader.  See the sample program (example.f, in this directory)
to see how the wrappers are used.

Include the statements
	implicit integer(a-z)
	double precision aduphys, getbasecount, getcfreq, sampfreq
(or the equivalent for your dialect of Fortran) in your Fortran program to
ensure that these functions (except for the four listed in the second
statement) will be understood to return integer*4 values (equivalent to C
`long' values).

Note that Fortran arrays are 1-based, and C arrays are 0-based.  Signal and
annotator numbers passed to these functions must be 0-based.

If you are using a UNIX Fortran compiler, or a Fortran-to-C translator, note
that the trailing `_' in these function names should *not* appear in your
Fortran program;  thus, for example, `annopen_' should be invoked as
`annopen'.  UNIX Fortran compilers and translators append a `_' to the
names of all external symbols referenced in Fortran source files when
generating object files.  Thus the linker can recognize that annopen_
(defined below) is the function required by a Fortran program that invokes
`annopen';  if the Fortran program were to invoke `annopen_', the linker
would search (unsuccessfully) for a function named `annopen__'.

If you are using a Fortran compiler that does not follow this convention,
you are on your own.
*/

#include <stdio.h>
#include <wfdb/wfdb.h>
#include <wfdb/ecgmap.h>
#ifndef BSD
# include <string.h>
#else		/* for Berkeley UNIX only */
# include <strings.h>
#endif

/* Define the C types equivalent to the corresponding Fortran types.
   The below definitions should be correct for modern systems; some
   older Fortran compilers may use different types. */
#ifndef INTEGER
#define INTEGER int
#endif
#ifndef DOUBLE_PRECISION
#define DOUBLE_PRECISION double
#endif
#ifndef STRINGSIZE
#define STRINGSIZE int
#endif

/* If Fortran strings are terminated by spaces rather than by null characters,
   define FIXSTRINGS. */
/* #define FIXSTRINGS */

/* If the size of a string is not specified, define NOSTRINGSIZE. */
/* #define NOSTRINGSIZE */

/* Convert a Fortran string (passed as a function or subroutine
   argument) to a C string. */
static char *fcstring(char **dest, const char *src, STRINGSIZE size)
{
    size_t len = size;

#ifdef NOSTRINGSIZE
# ifdef FIXSTRINGS
    /* NOSTRINGSIZE && FIXSTRINGS:
       - Buffer size is unknown.
       - The input string is terminated by the first space or null
         character. */
    len = strcspn(src, " ");
# else
    /* NOSTRINGSIZE && !FIXSTRINGS:
       - Buffer size is unknown.
       - The input string is terminated by the first null character. */
    len = strlen(src);
# endif
#else
# ifdef FIXSTRINGS
    /* !NOSTRINGSIZE && FIXSTRINGS:
       - Buffer size is specified as a hidden parameter.
       - The input string is terminated by the first null character or
         the last non-space character, or the end of the buffer. */
    while (len > 0 && src[len - 1] == ' ')
	len--;
# else
    /* !NOSTRINGSIZE && !FIXSTRINGS:
       - Buffer size is specified as a hidden parameter.
       - The input string is terminated by the first null character,
         or the end of the buffer. */
    const char *p = memchr(src, 0, len);
    if (p)
	len = p - src;
# endif
#endif

    SALLOC(*dest, len + 1, sizeof(char));
    if (*dest && len > 0)
	memcpy(*dest, src, len);
    return (*dest);
}

/* Convert a Fortran string to a C string; convert an empty string to NULL. */
static char *fcstring0(char **dest, const char *src, STRINGSIZE size)
{
    fcstring(dest, src, size);
    if (*dest && (*dest)[0] == 0)
	SFREE(*dest);
    return (*dest);
}

/* Convert a Fortran string to an 'aux' string. */
static void fastring(unsigned char **dest, const char *src, STRINGSIZE size)
{
    char *s = NULL;
    STRINGSIZE n;

    fcstring(&s, src, size);
    if (s && s[0]) {
	n = strlen(s);
	if (n > 255)
	    n = 255;
	SALLOC(*dest, n + 2, sizeof(char));
	if (*dest) {
	    (*dest)[0] = n;
	    memcpy(*dest + 1, s, n);
	}
    }
    else {
	SFREE(*dest);
    }
    SFREE(s);
}

/* Convert a C string to a Fortran string. */
static void cfstring(char *dest, STRINGSIZE size, const char *src)
{
    size_t max = size;
    size_t len;

    if (!src)
	src = "";
    len = strlen(src);

#ifdef NOSTRINGSIZE
# ifdef FIXSTRINGS
    /* NOSTRINGSIZE && FIXSTRINGS:
       - Buffer size is unknown.  If the length of the string exceeds
         the size of the buffer, the program will probably crash or
         misbehave.
       - The output string will be terminated by a single space
         character. */
    memcpy(dest, src, len);
    dest[len] = ' ';
# else
    /* NOSTRINGSIZE && !FIXSTRINGS:
       - Buffer size is unknown.  If the length of the string exceeds
         the size of the buffer, the program will probably crash or
         misbehave.
       - The output string will be terminated by a single null
         character. */
    memcpy(dest, src, len + 1);
# endif
#else
# ifdef FIXSTRINGS
    /* !NOSTRINGSIZE && FIXSTRINGS:
       - Buffer size is specified as a hidden parameter.  If the
         length of the string exceeds the size of the buffer, the
         string will be truncated.
       - The output string will be padded with space characters. */
    if (len >= max) {
	memcpy(dest, src, max);
    }
    else {
	memcpy(dest, src, len);
	memset(dest + len, ' ', max - len);
    }
# else
    /* !NOSTRINGSIZE && !FIXSTRINGS:
       - Buffer size is specified as a hidden parameter.  If the
         length of the string exceeds the size of the buffer, the
         string will be truncated.
       - The output string will be terminated by a single null
         character.  The remainder of the buffer will not be
         modified. */
    if (len >= max)
	len = max - 1;
    memcpy(dest, src, len);
    dest[len] = 0;
# endif
#endif
}

/* Static data shared by the wrapper functions.  Since Fortran does not support
   composite data types (such as C `struct' types), these are defined here, and
   functions for setting and retrieving data from these structures are provided
   below. */

static WFDB_Siginfo sinfo[WFDB_MAXSIG];
static WFDB_Calinfo cinfo;
static WFDB_Anninfo ainfo[WFDB_MAXANN*2];
static char *s1, *s2;

/* The functions setanninfo_, setsiginfo_, and getsiginfo_ do not have direct
   equivalents in the WFDB library;  they are provided in order to permit
   Fortran programs to read and write data structures passed to and from
   several of the WFDB library functions.  Since the contents of these
   structures are directly accessible by C programs, these functions are not
   needed in the C library.
*/

INTEGER setanninfo_(INTEGER *a, char *name, INTEGER *stat, STRINGSIZE name_size)
{
    if (0 <= *a && *a < WFDB_MAXANN*2) {
	fcstring(&ainfo[*a].name, name, name_size);
	ainfo[*a].stat = *stat;
	return (0L);
    }
    else
	return (-1L);
}

INTEGER getsiginfo_(INTEGER *s, char *fname, char *desc, char *units, DOUBLE_PRECISION *gain, INTEGER *initval, INTEGER *group, INTEGER *fmt, INTEGER *spf, INTEGER *bsize, INTEGER *adcres, INTEGER *adczero, INTEGER *baseline, INTEGER *nsamp, INTEGER *cksum, STRINGSIZE fname_size, STRINGSIZE desc_size, STRINGSIZE units_size)
{
    if (0 <= *s && *s < WFDB_MAXSIG) {
	cfstring(fname, fname_size, sinfo[*s].fname);
	cfstring(desc, desc_size, sinfo[*s].desc);
	cfstring(units, units_size, sinfo[*s].units);
	*gain = sinfo[*s].gain;
	*initval = sinfo[*s].initval;
	*group = sinfo[*s].group;
	*fmt = sinfo[*s].fmt;
	*spf = sinfo[*s].spf;
	*bsize = sinfo[*s].bsize;
	*adcres = sinfo[*s].adcres;
	*adczero = sinfo[*s].adczero;
	*baseline = sinfo[*s].baseline;
	*nsamp = sinfo[*s].nsamp;
	*cksum = sinfo[*s].cksum;
	return (0L);
    }
    else
	return (-1L);
}

INTEGER setsiginfo_(INTEGER *s, char *fname, char *desc, char *units, DOUBLE_PRECISION *gain, INTEGER *initval, INTEGER *group, INTEGER *fmt, INTEGER *spf, INTEGER *bsize, INTEGER *adcres, INTEGER *adczero, INTEGER *baseline, INTEGER *nsamp, INTEGER *cksum, STRINGSIZE fname_size, STRINGSIZE desc_size, STRINGSIZE units_size)
{
    if (0 <= *s && *s < WFDB_MAXSIG) {
	sinfo[*s].fname = fname;
	sinfo[*s].desc = desc;
	sinfo[*s].units = units;
	sinfo[*s].gain = *gain;
	sinfo[*s].initval = *initval;
	sinfo[*s].group = *group;
	sinfo[*s].fmt = *fmt;
	sinfo[*s].spf = *spf;
	sinfo[*s].bsize = *bsize;
	sinfo[*s].adcres = *adcres;
	sinfo[*s].adczero = *adczero;
	sinfo[*s].baseline = *baseline;
	sinfo[*s].nsamp = *nsamp;
	sinfo[*s].cksum = *cksum;
	return (0L);
    }
    else
	return (-1L);
}

/* Before using annopen_, set up the annotation information structures using
   setanninfo_. */
INTEGER annopen_(char *record, INTEGER *nann, STRINGSIZE record_size)
{
    return (annopen(fcstring(&s1, record, record_size),
		    ainfo, (unsigned int)(*nann)));
}

/* After using isigopen_ or osigopen_, use getsiginfo_ to obtain the contents
   of the signal information structures if necessary. */
INTEGER isigopen_(char *record, INTEGER *nsig, STRINGSIZE record_size)
{
    return (isigopen(fcstring(&s1, record, record_size),
		     sinfo, (unsigned int)(*nsig)));
}

INTEGER osigopen_(char *record, INTEGER *nsig, STRINGSIZE record_size)
{
    return (osigopen(fcstring(&s1, record, record_size),
		     sinfo, (unsigned int)(*nsig)));
}

/* Before using osigfopen_, use setsiginfo_ to set the contents of the signal
   information structures. */
INTEGER osigfopen_(INTEGER *nsig)
{
    return (osigfopen(sinfo, (unsigned int)(*nsig)));
}

/* Before using wfdbinit_, use setanninfo_ and setsiginfo_ to set the contents
   of the annotation and signal information structures. */
INTEGER wfdbinit_(char *record, INTEGER *nann, INTEGER *nsig, STRINGSIZE record_size)
{
    return (wfdbinit(fcstring(&s1, record, record_size),
		     ainfo, (unsigned int)(*nann),
		     sinfo, (unsigned int)(*nsig)));
}

INTEGER findsig_(char *signame, STRINGSIZE signame_size)
{
    return (findsig(fcstring(&s1, signame, signame_size)));
}

INTEGER getspf_(INTEGER *dummy)
{
    return (getspf());
}

INTEGER setgvmode_(INTEGER *mode)
{
    setgvmode((int)(*mode));
    return (0L);
}

INTEGER getgvmode_(INTEGER *dummy)
{
    return (getgvmode());
}

INTEGER setifreq_(DOUBLE_PRECISION *freq)
{
    return (setifreq((WFDB_Frequency)*freq));
}

DOUBLE_PRECISION getifreq_(INTEGER *dummy)
{
    return (getifreq());
}

INTEGER getvec_(INTEGER *long_vector)
{
    if (sizeof(WFDB_Sample) == sizeof(INTEGER)) {
	return (getvec((WFDB_Sample *)long_vector));
    }
    else {
	WFDB_Sample v[WFDB_MAXSIG];
	int i, j;

	i = getvec(v);
	for (j = 0; j < i; j++)
	    long_vector[i] = v[i];
	return (i);
    }
}

INTEGER getframe_(INTEGER *long_vector)
{
    if (sizeof(WFDB_Sample) == sizeof(INTEGER)) {
	return (getframe((WFDB_Sample *)long_vector));
    }
    else {
	WFDB_Sample v[WFDB_MAXSIG*WFDB_MAXSPF];
	int i, j, k;

	i = getframe(v);
	for (j = 0; j < i; j += k)
	    for (k = 0; k < sinfo[i].spf; k++)
		long_vector[j+k] = v[j+k];
	return (i);
    }
}

INTEGER putvec_(INTEGER *long_vector)
{
    if (sizeof(WFDB_Sample) == sizeof(INTEGER)) {
	return (putvec((WFDB_Sample *)long_vector));
    }
    else {
	WFDB_Sample v[WFDB_MAXSIG*WFDB_MAXSPF];
	int i;

	for (i = 0; i < WFDB_MAXSIG*WFDB_MAXSPF; i++)
	    v[i] = long_vector[i];
	return (putvec(v));
    }
}

INTEGER getann_(INTEGER *annotator, INTEGER *time, INTEGER *anntyp, INTEGER *subtyp, INTEGER *chan, INTEGER *num, char *aux, STRINGSIZE aux_size)
{
    static WFDB_Annotation iann;
    int i, j;

    i = getann((WFDB_Annotator)(*annotator), &iann);
    if (i == 0) {
	*time = iann.time;
	*anntyp = iann.anntyp;
	*subtyp = iann.subtyp;
	*chan = iann.chan;
	*num = iann.num;
	aux = iann.aux;
    }
    return (i);
}

INTEGER ungetann_(INTEGER *annotator, INTEGER *time, INTEGER *anntyp, INTEGER *subtyp, INTEGER *chan, INTEGER *num, char *aux, STRINGSIZE aux_size)
{
    static WFDB_Annotation oann;
    int i, j;

    oann.time = *time;
    oann.anntyp = *anntyp;
    oann.subtyp = *subtyp;
    oann.chan = *chan;
    oann.num = *num;
    oann.aux = aux;
    return (ungetann((WFDB_Annotator)(*annotator), &oann));
}

INTEGER putann_(INTEGER *annotator, INTEGER *time, INTEGER *anntyp, INTEGER *subtyp, INTEGER *chan, INTEGER *num, char *aux, STRINGSIZE aux_size)
{
    static WFDB_Annotation oann;
    int i, j;

    oann.time = *time;
    oann.anntyp = *anntyp;
    oann.subtyp = *subtyp;
    oann.chan = *chan;
    oann.num = *num;
    fastring(&oann.aux, aux, aux_size);
    return (putann((WFDB_Annotator)(*annotator), &oann));
}

INTEGER isigsettime_(INTEGER *time)
{
    return (isigsettime((WFDB_Time)(*time)));
}

INTEGER isgsettime_(INTEGER *group, INTEGER *time)
{
    return (isgsettime((WFDB_Group)(*group), (WFDB_Time)(*time)));
}

INTEGER tnextvec_(INTEGER *signal, INTEGER *time)
{
    return (tnextvec((WFDB_Signal)(*signal), (WFDB_Time)(*time)));
} 

INTEGER iannsettime_(INTEGER *time)
{
    return (iannsettime((WFDB_Time)(*time)));
}

INTEGER ecgstr_(INTEGER *code, char *string, STRINGSIZE string_size)
{
    cfstring(string, string_size, ecgstr((int)(*code)));
    return (0L);
}

INTEGER strecg_(char *string, STRINGSIZE string_size)
{
    return (strecg(fcstring(&s1, string, string_size)));
}

INTEGER setecgstr_(INTEGER *code, char *string, STRINGSIZE string_size)
{
    return (setecgstr((int)(*code), fcstring(&s1, string, string_size)));
}

INTEGER annstr_(INTEGER *code, char *string, STRINGSIZE string_size)
{
    cfstring(string, string_size, annstr((int)(*code)));
    return (0L);
}

INTEGER strann_(char *string, STRINGSIZE string_size)
{
    return (strann(fcstring(&s1, string, string_size)));
}

INTEGER setannstr_(INTEGER *code, char *string, STRINGSIZE string_size)
{
    return (setannstr((int)(*code), fcstring(&s1, string, string_size)));
}

INTEGER anndesc_(INTEGER *code, char *string, STRINGSIZE string_size)
{
    cfstring(string, string_size, anndesc((int)(*code)));
    return (0L);
}

INTEGER setanndesc_(INTEGER *code, char *string, STRINGSIZE string_size)
{
    return (setanndesc((int)(*code), fcstring(&s1, string, string_size)));
}

INTEGER setafreq_(DOUBLE_PRECISION *freq)
{
    setafreq(*freq);
    return (0L);
}

DOUBLE_PRECISION getafreq_(INTEGER *dummy)
{
    return (getafreq());
}

INTEGER setiafreq_(INTEGER *annotator, DOUBLE_PRECISION *freq)
{
    setiafreq(*annotator, *freq);
    return (0L);
}

DOUBLE_PRECISION getiafreq_(INTEGER *annotator)
{
    return (getiafreq(*annotator));
}

DOUBLE_PRECISION getiaorigfreq_(INTEGER *annotator)
{
    return (getiaorigfreq(*annotator));
}

INTEGER iannclose_(INTEGER *annotator)
{
    iannclose((WFDB_Annotator)(*annotator));
    return (0L);
}

INTEGER oannclose_(INTEGER *annotator)
{
    oannclose((WFDB_Annotator)(*annotator));
    return (0L);
}

/* The functions below can be used in place of the macros defined in
   <wfdb/ecgmap.h>. */

INTEGER isann_(INTEGER *anntyp)
{   
    return (wfdb_isann((int)*anntyp));
}

INTEGER isqrs_(INTEGER *anntyp)
{   
    return (wfdb_isqrs((int)*anntyp));
}

INTEGER setisqrs_(INTEGER *anntyp, INTEGER *value)
{   
    wfdb_setisqrs((int)*anntyp, (int)*value);
    return (0L);
}

INTEGER map1_(INTEGER *anntyp)
{   
    return (wfdb_map1((int)*anntyp));
}

INTEGER setmap1_(INTEGER *anntyp, INTEGER *value)
{   
    wfdb_setmap1((int)*anntyp, (int)*value);
    return (0L);
}

INTEGER map2_(INTEGER *anntyp)
{   
    return (wfdb_map2((int)*anntyp));
}

INTEGER setmap2_(INTEGER *anntyp, INTEGER *value)
{   
    wfdb_setmap2((int)*anntyp, (int)*value);
    return (0L);
}

INTEGER ammap_(INTEGER *anntyp)
{   
    return (wfdb_ammap((int)*anntyp));
}

INTEGER mamap_(INTEGER *anntyp, INTEGER *subtyp)
{   
    return (wfdb_mamap((int)*anntyp, (int)*subtyp));
}

INTEGER annpos_(INTEGER *anntyp)
{   
    return (wfdb_annpos((int)*anntyp));
}

INTEGER setannpos_(INTEGER *anntyp, INTEGER *value)
{   
    wfdb_setannpos((int)*anntyp, (int)*value);
    return (0L);
}

INTEGER timstr_(INTEGER *time, char *string, STRINGSIZE string_size)
{
    cfstring(string, string_size, timstr((WFDB_Time)(*time)));
    return (0L);
}

INTEGER mstimstr_(INTEGER *time, char *string, STRINGSIZE string_size)
{
    cfstring(string, string_size, mstimstr((WFDB_Time)(*time)));
    return (0L);
}

INTEGER strtim_(char *string, STRINGSIZE string_size)
{
    return (strtim(fcstring(&s1, string, string_size)));
}

INTEGER datstr_(INTEGER *date, char *string, STRINGSIZE string_size)
{
    cfstring(string, string_size, datstr((WFDB_Date)(*date)));
    return (0L);
}

INTEGER strdat_(char *string, STRINGSIZE string_size)
{
    return (strdat(fcstring(&s1, string, string_size)));
}

INTEGER adumuv_(INTEGER *signal, INTEGER *ampl)
{
    return (adumuv((WFDB_Signal)(*signal), (WFDB_Sample)(*ampl)));
}

INTEGER muvadu_(INTEGER *signal, INTEGER *microvolts)
{
    return (muvadu((WFDB_Signal)(*signal), (int)(*microvolts)));
}

DOUBLE_PRECISION aduphys_(INTEGER *signal, INTEGER *ampl)
{
    return (aduphys((WFDB_Signal)(*signal), (WFDB_Sample)(*ampl)));
}

INTEGER physadu_(INTEGER *signal, DOUBLE_PRECISION *v)
{
    return (physadu((WFDB_Signal)(*signal), *v));
}

INTEGER sample_(INTEGER *signal, INTEGER *t)
{
    return (sample((WFDB_Signal)(*signal), (WFDB_Time)(*t)));
}

INTEGER sample_valid_(INTEGER *dummy)
{
    return (sample_valid());
}

INTEGER calopen_(char *filename, STRINGSIZE filename_size)
{
    return (calopen(fcstring(&s1, filename, filename_size)));
}

INTEGER getcal_(char *description, char *units, DOUBLE_PRECISION *low, DOUBLE_PRECISION *high, DOUBLE_PRECISION *scale, INTEGER *caltype, STRINGSIZE description_size, STRINGSIZE units_size)
{
    if (getcal(fcstring(&s1, description, description_size),
	       fcstring(&s2, units, units_size), &cinfo) == 0) {
	*low = cinfo.low;
	*high = cinfo.high;
	*scale = cinfo.scale;
	*caltype = cinfo.caltype;
	return (0L);
    }
    else
	return (-1L);
}

INTEGER putcal_(char *description, char *units, DOUBLE_PRECISION *low, DOUBLE_PRECISION *high, DOUBLE_PRECISION *scale, INTEGER *caltype, STRINGSIZE description_size, STRINGSIZE units_size)
{
    cinfo.sigtype = fcstring(&s1, description, description_size);
    cinfo.units = fcstring(&s2, units, units_size);
    cinfo.low = *low;
    cinfo.high = *high;
    cinfo.scale = *scale;
    cinfo.caltype = *caltype;
    return (putcal(&cinfo));
}

INTEGER newcal_(char *filename, STRINGSIZE filename_size)
{
    return (newcal(fcstring(&s1, filename, filename_size)));
}

INTEGER flushcal_(INTEGER *dummy)
{
    flushcal();
    return (0L);
}

INTEGER getinfo_(char *record, char *string, STRINGSIZE record_size, STRINGSIZE string_size)
{
    char *s = getinfo(fcstring(&s1, record, record_size));
    cfstring(string, string_size, s);
    return (0L);
}

INTEGER putinfo_(char *string, STRINGSIZE string_size)
{
    return (putinfo(fcstring(&s1, string, string_size)));
}

INTEGER setinfo_(char *record, STRINGSIZE record_size)
{
    return (setinfo(fcstring(&s1, record, record_size)));
}

INTEGER wfdb_freeinfo_(INTEGER *dummy)
{
    wfdb_freeinfo();
    return (0L);
}

INTEGER newheader_(char *record, STRINGSIZE record_size)
{
    return (newheader(fcstring(&s1, record, record_size)));
}

/* Before using setheader_, use setsiginfo to set the contents of the signal
   information structures. */
INTEGER setheader_(char *record, INTEGER *nsig, STRINGSIZE record_size)
{
    return (setheader(fcstring(&s1, record, record_size),
		      sinfo, (unsigned int)(*nsig)));
}

/* No wrappers are provided for setmsheader or getseginfo. */

INTEGER wfdbgetskew_(INTEGER *s)
{
    return (wfdbgetskew((WFDB_Signal)(*s)));
}

INTEGER wfdbsetiskew_(INTEGER *s, INTEGER *skew)
{
    wfdbsetiskew((WFDB_Signal)(*s), (int)(*skew));
    return (0L);
}

INTEGER wfdbsetskew_(INTEGER *s, INTEGER *skew)
{
    wfdbsetskew((WFDB_Signal)(*s), (int)(*skew));
    return (0L);
}

INTEGER wfdbgetstart_(INTEGER *s)
{
    return (wfdbgetstart((WFDB_Signal)(*s)));
}

INTEGER wfdbsetstart_(INTEGER *s, INTEGER *bytes)
{
    wfdbsetstart((WFDB_Signal)(*s), *bytes);
    return (0L);
}

INTEGER wfdbputprolog_(char *prolog, INTEGER *bytes, INTEGER *signal, STRINGSIZE prolog_size)
{
    return (wfdbputprolog(prolog, (long)*bytes,(WFDB_Signal)*signal));
}

INTEGER wfdbquit_(INTEGER *dummy)
{
    wfdbquit();
    return (0L);
}

DOUBLE_PRECISION sampfreq_(char *record, STRINGSIZE record_size)
{
    return (sampfreq(fcstring(&s1, record, record_size)));
}

INTEGER setsampfreq_(DOUBLE_PRECISION *frequency)
{
    return (setsampfreq((WFDB_Frequency)(*frequency)));
}

DOUBLE_PRECISION getcfreq_(INTEGER *dummy)
{
    return (getcfreq());
}

INTEGER setcfreq_(DOUBLE_PRECISION *frequency)
{
    setcfreq((WFDB_Frequency)(*frequency));
    return (0L);
}

DOUBLE_PRECISION getbasecount_(INTEGER *dummy)
{
    return (getbasecount());
}

INTEGER setbasecount_(DOUBLE_PRECISION *count)
{
    setbasecount(*count);
    return (0L);
}

INTEGER setbasetime_(char *string, STRINGSIZE string_size)
{
    return (setbasetime(fcstring(&s1, string, string_size)));
}

INTEGER wfdbquiet_(INTEGER *dummy)
{
    wfdbquiet();
    return (0L);
}

INTEGER wfdbverbose_(INTEGER *dummy)
{
    wfdbverbose();
    return (0L);
}

INTEGER wfdberror_(char *string, STRINGSIZE string_size)
{
    cfstring(string, string_size, wfdberror());
    return (0L);
}

INTEGER setwfdb_(char *string, STRINGSIZE string_size)
{
    setwfdb(fcstring(&s1, string, string_size));
    return (0L);
}

INTEGER getwfdb_(char *string, STRINGSIZE string_size)
{
    cfstring(string, string_size, getwfdb());
    return (0L);
}

INTEGER resetwfdb_(INTEGER *dummy)
{
    resetwfdb();
    return (0L);
}

INTEGER setibsize_(INTEGER *input_buffer_size)
{
    return (setibsize((int)(*input_buffer_size)));
}

INTEGER setobsize_(INTEGER *output_buffer_size)
{
    return (setobsize((int)(*output_buffer_size)));
}

INTEGER wfdbfile_(char *file_type, char *record, char *pathname, STRINGSIZE file_type_size, STRINGSIZE record_size, STRINGSIZE pathname_size)
{
    char *s = wfdbfile(fcstring(&s1, file_type, file_type_size),
		       fcstring(&s2, record, record_size));
    cfstring(pathname, pathname_size, s);
    return (0L);
}

INTEGER wfdbflush_(INTEGER *dummy)
{
    wfdbflush();
    return (0L);
}

INTEGER wfdbmemerr_(INTEGER *exit_if_error)
{
    wfdbmemerr((int)(*exit_if_error));
    return (0L);
}

INTEGER wfdbversion_(char *version, STRINGSIZE version_size)
{
    cfstring(version, version_size, wfdbversion());
    return (0L);
}

INTEGER wfdbldflags_(char *ldflags, STRINGSIZE ldflags_size)
{
    cfstring(ldflags, ldflags_size, wfdbldflags());
    return (0L);
}

INTEGER wfdbcflags_(char *cflags, STRINGSIZE cflags_size)
{
    cfstring(cflags, cflags_size, wfdbcflags());
    return (0L);
}

INTEGER wfdbdefwfdb_(char *defwfdb, STRINGSIZE defwfdb_size)
{
    cfstring(defwfdb, defwfdb_size, wfdbdefwfdb());
    return (0L);
}

INTEGER wfdbdefwfdbcal_(char *defwfdbcal, STRINGSIZE defwfdbcal_size)
{
    cfstring(defwfdbcal, defwfdbcal_size, wfdbdefwfdbcal());
    return (0L);
}
