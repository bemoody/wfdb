/* file: wfdbf.c	G. Moody	23 August 1995
			Last revised:  6 February 2002

_______________________________________________________________________________
wfdbf: Fortran wrappers for the WFDB library functions
Copyright (C) 2002 George B. Moody

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

General notes on using the wrapper functions

Wrappers are provided below for all of the WFDB library functions except
setmsheader.  See the sample program (example.f, in this directory)
to see how the wrappers are used.

Include the statements
	implicit integer(a-z)
	real aduphys, getbasecount, getcfreq, sampfreq
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
generating object files.  Thus the linker can recognize that annopen1_
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

/* If a WFDB_Sample is the same size as a long (true except for MS-DOS PCs and
   a few less common systems), considerable efficiency gains are possible.
   Otherwise, define REPACK below to compile the code needed to repack arrays
   of DB_Samples into long arrays. */
/* #define REPACK */

/* If Fortran strings are terminated by spaces rather than by null characters,
   define FIXSTRINGS. */
/* #define FIXSTRINGS */

#ifdef FIXSTRINGS
fcstring(s)	/* change final space to null */
char *s;
{
    while (*s && *s != ' ')
	s++;
    *s = '\0';
}

cfstring(s)	/* change final null to space */
char *s;
{
    while (*s)
	s++;
    *s = ' ';
}
#else
#define fcstring(s)
#define cfstring(s)
#endif

/* Static data shared by the wrapper functions.  Since Fortran does not support
   composite data types (such as C `struct' types), these are defined here, and
   functions for setting and retrieving data from these structures are provided
   below. */

static WFDB_Siginfo sinfo[WFDB_MAXSIG];
static WFDB_Calinfo cinfo;
static WFDB_Anninfo ainfo[WFDB_MAXANN*2];

/* The functions setanninfo_, setsiginfo_, and getsiginfo_ do not have direct
   equivalents in the WFDB library;  they are provided in order to permit
   Fortran programs to read and write data structures passed to and from
   several of the WFDB library functions.  Since the contents of these
   structures are directly accessible by C programs, these functions are not
   needed in the C library.
*/

long setanninfo_(a, name, stat)
long *a;
char *name;
long *stat;
{
    if (0 <= *a && *a < WFDB_MAXANN*2) {
	fcstring(name);
	ainfo[*a].name = name;
	ainfo[*a].stat = *stat;
	return (0L);
    }
    else
	return (-1L);
}

long getsiginfo_(s, fname, desc, units, gain, initval, group, fmt, spf,
		 bsize, adcres, adczero, baseline, nsamp, cksum)
long *s;
char *fname, *desc, *units;
double *gain;
long *initval, *group, *fmt, *spf, *bsize, *adcres, *adczero, *baseline,
     *nsamp, *cksum;
{
    if (0 <= *s && *s < WFDB_MAXSIG) {
	fname = sinfo[*s].fname;
	desc = sinfo[*s].desc;
	units = sinfo[*s].units;
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

long setsiginfo_(s, fname, desc, units, gain, initval, group, fmt, spf,
		 bsize, adcres, adczero, baseline, nsamp, cksum)
long *s;
char *fname, *desc, *units;
double *gain;
long *initval, *group, *fmt, *spf, *bsize, *adcres, *adczero, *baseline,
     *nsamp, *cksum;
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
long annopen_(record, nann)
char *record;
long *nann;
{
    fcstring(record);
    return (annopen(record, ainfo, (unsigned int)(*nann)));
}

/* After using isigopen_ or osigopen_, use getsiginfo_ to obtain the contents
   of the signal information structures if necessary. */
long isigopen_(record, nsig)
char *record;
long *nsig;
{
    fcstring(record);
    return (isigopen(record, sinfo, (unsigned int)(*nsig)));
}

long osigopen_(record, nsig)
char *record;
long *nsig;
{
    fcstring(record);
    return (osigopen(record, sinfo, (unsigned int)(*nsig)));
}

/* Before using osigfopen_, use setsiginfo to set the contents of the signal
   information structures. */
long osigfopen_(nsig)
long *nsig;
{
    return (osigfopen(sinfo, (unsigned int)(*nsig)));
}

/* Before using wfdbinit_, use setanninfo and setsiginfo to set the contents of
   the annotation and signal information structures. */
long wfdbinit_(record, nann, nsig)
char *record;
long *nann, *nsig;
{
    fcstring(record);
    return (wfdbinit(record, ainfo, (unsigned int)(*nann),
		           sinfo, (unsigned int)(*nsig)));
}

long setgvmode_(mode)
long *mode;
{
    setgvmode((int)(*mode));
    return (0L);
}

long getspf_(void)
{
    return (getspf());
}

long getvec_(long_vector)
long *long_vector;
{
#ifndef REPACK
    return (getvec((WFDB_Sample *)long_vector));
#else
    WFDB_Sample v[WFDB_MAXSIG];
    int i, j;

    i = getvec(v);
    for (j = 0; j < i; j++)
	long_vector[i] = v[i];
    return (i);
#endif
}

long getframe_(long_vector)
long *long_vector;
{
#ifndef REPACK
    return (getframe((WFDB_Sample *)long_vector));
#else
    WFDB_Sample v[WFDB_MAXSIG*WFDB_MAXSPF];
    int i, j, k;

    i = getframe(v);
    for (j = 0; j < i; j += k)
	for (k = 0; k < sinfo[i].spf; k++)
	    long_vector[j+k] = v[j+k];
    return (i);
#endif
}

long putvec_(long_vector)
long *long_vector;
{
#ifndef REPACK
    return (putvec((WFDB_Sample *)long_vector));
#else
    WFDB_Sample v[WFDB_MAXSIG*WFDB_MAXSPF];
    int i;

    for (i = 0; i < WFDB_MAXSIG*WFDB_MAXSPF; i++)
	v[i] = long_vector[i];
    return (putvec(v));
#endif
}

long getann_(annotator, time, anntyp, subtyp, chan, num, aux)
long *annotator, *time, *anntyp, *subtyp, *chan, *num;
char *aux;
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

long ungetann_(annotator, time, anntyp, subtyp, chan, num, aux)
long *annotator, *time, *anntyp, *subtyp, *chan, *num;
char *aux;
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

long putann_(annotator, time, anntyp, subtyp, chan, num, aux)
long *annotator, *time, *anntyp, *subtyp, *chan, *num;
char *aux;
{
    static WFDB_Annotation oann;
    int i, j;

    oann.time = *time;
    oann.anntyp = *anntyp;
    oann.subtyp = *subtyp;
    oann.chan = *chan;
    oann.num = *num;
    oann.aux = aux;
    return (putann((WFDB_Annotator)(*annotator), &oann));
}

long isigsettime_(time)
long *time;
{
    return (isigsettime((WFDB_Time)(*time)));
}

long isgsettime_(group, time)
long *group, *time;
{
    return (isgsettime((WFDB_Group)(*group), (WFDB_Time)(*time)));
}

long iannsettime_(time)
long *time;
{
    return (iannsettime((WFDB_Time)(*time)));
}

long ecgstr_(code, string)
long *code;
char *string;
{
    strcpy(string, ecgstr((int)(*code)));
    cfstring(string);
    return (0L);
}

long strecg_(string)
char *string;
{
    fcstring(string);
    return (strecg(string));
}

long setecgstr_(code, string)
long *code;
char *string;
{
    fcstring(string);
    return (setecgstr((int)(*code), string));
}

long annstr_(code, string)
long *code;
char *string;
{
    strcpy(string, annstr((int)(*code)));
    cfstring(string);
    return (0L);
}

long strann_(string)
char *string;
{
    fcstring(string);
    return (strann(string));
}

long setannstr_(code, string)
long *code;
char *string;
{
    fcstring(string);
    return (setannstr((int)(*code), string));
}

long anndesc_(code, string)
long *code;
char *string;
{
    strcpy(string, anndesc((int)(*code)));
    cfstring(string);
    return (0L);
}

long setanndesc_(code, string)
long *code;
char *string;
{
    fcstring(string);
    return (setanndesc((int)(*code), string));
}

long iannclose_(annotator)
long *annotator;
{
    iannclose((WFDB_Annotator)(*annotator));
    return (0L);
}

long oannclose_(annotator)
long *annotator;
{
    oannclose((WFDB_Annotator)(*annotator));
    return (0L);
}

long timstr_(time, string)
long *time;
char *string;
{
    strcpy(string, timstr((WFDB_Time)(*time)));
    cfstring(string);
    return (0L);
}

long mstimstr_(time, string)
long *time;
char *string;
{
    strcpy(string, mstimstr((WFDB_Time)(*time)));
    cfstring(string);
    return (0L);
}

long strtim_(string)
char *string;
{
    fcstring(string);
    return (strtim(string));
}

long datstr_(date, string)
long *date;
char *string;
{
    strcpy(string, datstr((WFDB_Date)(*date)));
    cfstring(string);
    return (0L);
}

long strdat_(string)
char *string;
{
    fcstring(string);
    return (strdat(string));
}

long adumuv_(signal, ampl)
long *signal, *ampl;
{
    return (adumuv((WFDB_Signal)(*signal), (WFDB_Sample)(*ampl)));
}

long muvadu_(signal, microvolts)
long *signal, *microvolts;
{
    return (muvadu((WFDB_Signal)(*signal), (int)(*microvolts)));
}

double aduphys_(signal, ampl)
long *signal, *ampl;
{
    return (aduphys((WFDB_Signal)(*signal), (WFDB_Sample)(*ampl)));
}

long physadu_(signal, v)
long *signal;
double *v;
{
    return (physadu((WFDB_Signal)(*signal), *v));
}

long calopen_(calibration_filename)
char *calibration_filename;
{
    fcstring(calibration_filename);
    return (calopen(calibration_filename));
}

long getcal_(description, units, low, high, scale, caltype)
char *description, *units;
double *low, *high, *scale;
long *caltype;
{
    fcstring(description);
    fcstring(units);
    if (getcal(description, units, &cinfo) == 0) {
	*low = cinfo.low;
	*high = cinfo.high;
	*scale = cinfo.scale;
	*caltype = cinfo.caltype;
	return (0L);
    }
    else
	return (-1L);
}

long putcal_(description, units, low, high, scale, caltype)
char *description, *units;
double *low, *high, *scale;
long *caltype;
{
    fcstring(description);
    fcstring(units);
    cinfo.sigtype = description;
    cinfo.units = units;
    cinfo.low = *low;
    cinfo.high = *high;
    cinfo.scale = *scale;
    cinfo.caltype = *caltype;
    return (putcal(&cinfo));
}

long newcal_(calibration_filename)
char *calibration_filename;
{
    fcstring(calibration_filename);
    return (newcal(calibration_filename));
}

long flushcal_(dummy)
long *dummy;
{
    flushcal();
    return (0L);
}

long getinfo_(record, string)
char *record, *string;
{
    fcstring(record);
    strcpy(string, getinfo(record));
    cfstring(string);
    return (0L);
}

long putinfo_(string)
char *string;
{
    fcstring(string);
    return (putinfo(string));
}

long newheader_(record)
char *record;
{
    fcstring(record);
    return (newheader(record));
}

/* Before using setheader_, use setsiginfo to set the contents of the signal
   information structures. */
long setheader_(record, nsig)
char *record;
long *nsig;
{
    fcstring(record);
    return (setheader(record, sinfo, (unsigned int)(*nsig)));
}

/* No wrapper is provided for setmsheader. */

long wfdbgetskew_(s)
long *s;
{
    return (wfdbgetskew((WFDB_Signal)(*s)));
}

long wfdbsetskew_(s, skew)
long *s, *skew;
{
    wfdbsetskew((WFDB_Signal)(*s), (int)(*skew));
    return (0L);
}

long wfdbgetstart_(s)
long *s;
{
    return (wfdbgetstart((WFDB_Signal)(*s)));
}

long wfdbsetstart_(s, bytes)
long *s, *bytes;
{
    wfdbsetstart((WFDB_Signal)(*s), *bytes);
    return (0L);
}

long wfdbquit_(dummy)
long *dummy;
{
    wfdbquit();
    return (0L);
}

double sampfreq_(record)
char *record;
{
    fcstring(record);
    return (sampfreq(record));
}

long setsampfreq_(frequency)
double *frequency;
{
    return (setsampfreq((WFDB_Frequency)(*frequency)));
}

double getcfreq_(dummy)
long *dummy;
{
    return (getcfreq());
}

long setcfreq_(frequency)
double *frequency;
{
    setcfreq((WFDB_Frequency)(*frequency));
    return (0L);
}

double getbasecount_(dummy)
long *dummy;
{
    return (getbasecount());
}

long setbasecount_(count)
double *count;
{
    setbasecount(*count);
    return (0L);
}

long setbasetime_(string)
char *string;
{
    fcstring(string);
    return (setbasetime(string));
}

long wfdbquiet_(dummy)
long *dummy;
{
    wfdbquiet();
    return (0L);
}

long wfdbverbose_(dummy)
long *dummy;
{
    wfdbverbose();
    return (0L);
}

long wfdberror_(string)
char *string;
{
    strcpy(string, wfdberror());
    cfstring(string);
    return (0L);
}

long setwfdb_(string)
char *string;
{
    fcstring(string);
    setwfdb(string);
    return (0L);
}

long getwfdb_(string)
char *string;
{
    strcpy(string, getwfdb());
    cfstring(string);
    return (0L);
}

long setibsize_(input_buffer_size)
long *input_buffer_size;
{
    return (setibsize((int)(*input_buffer_size)));
}

long setobsize_(output_buffer_size)
long *output_buffer_size;
{
    return (setobsize((int)(*output_buffer_size)));
}

long wfdbfile_(file_type, record, pathname)
char *file_type, *record, *pathname;
{
    fcstring(file_type);
    fcstring(record);
    strcpy(pathname, wfdbfile(file_type, record));
    cfstring(pathname);
    return (0L);
}

long wfdbflush_(dummy)
long *dummy;
{
    wfdbflush();
    return (0L);
}

/* The functions below can be used in place of the macros defined in
   <wfdb/ecgmap.h>. */

long isann_(anntyp)
long *anntyp;
{   
    return ((long)(isann(*anntyp)));
}

long isqrs_(anntyp)
long *anntyp;
{   
    return ((long)(isqrs(*anntyp)));
}

long setisqrs_(anntyp, value)
long *anntyp, *value;
{   
    setisqrs(*anntyp, *value);
    return (0L);
}

long map1_(anntyp)
long *anntyp;
{   
    return ((long)(map1(*anntyp)));
}

long setmap1_(anntyp, value)
long *anntyp, *value;
{   
    setmap1(*anntyp, *value);
    return (0L);
}

long map2_(anntyp)
long *anntyp;
{   
    return ((long)(map1(*anntyp)));
}

long setmap2_(anntyp, value)
long *anntyp, *value;
{   
    setmap1(*anntyp, *value);
    return (0L);
}

long ammap_(anntyp)
long *anntyp;
{   
    return ((long)(ammap(*anntyp)));
}

long mamap_(anntyp, subtyp)
long *anntyp, *subtyp;
{   
    return ((long)(mamap(*anntyp, *subtyp)));
}

long annpos_(anntyp)
long *anntyp;
{   
    return ((long)(annpos(*anntyp)));
}

long setannpos_(anntyp, value)
long *anntyp, *value;
{   
    setannpos(*anntyp, *value);
    return (0L);
}
