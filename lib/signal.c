/* file: signal.c	G. Moody	13 April 1989
			Last revised:   14 November 2004	wfdblib 10.3.14
WFDB library functions for signals

_______________________________________________________________________________
wfdb: a library for reading and writing annotated waveforms (time series data)
Copyright (C) 1989-2004 George B. Moody

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

This file contains definitions of the following functions, which are not
visible outside of this file:
 allocisig	(sets max number of simultaneously open input signals)
 allocigroup	(sets max number of simultaneously open input signal groups)
 allocosig	(sets max number of simultaneously open output signals)
 allocogroup	(sets max number of simultaneously open output signal groups)
 isfmt		(checks if argument is a legal signal format type)
 copysi		(deep-copies a WFDB_Siginfo structure)
 readheader	(reads a header file)
 hsdfree	(deallocates memory used by readheader)
 isigclose	(closes input signals)
 osigclose	(closes output signals)
 isgsetframe	(skips to a specified frame number in a specified signal group)
 getskewedframe	(reads an input frame, without skew correction)
 rgetvec        (reads a sample from each input signal without resampling)

This file also contains low-level I/O routines for signals in various formats;
typically, the input routine for format N signals is named rN(), and the output
routine is named wN().  Most of these routines are implemented as macros for
efficiency.  These low-level I/O routines are not visible outside of this file.

This file also contains definitions of the following WFDB library functions:
 isigopen	(opens input signals)
 osigopen	(opens output signals)
 osigfopen	(opens output signals by name)
 getspf [9.6]	(returns number of samples returned by getvec per frame)
 setgvmode [9.0](sets getvec operating mode)
 setifreq [10.2.6](sets the getvec sampling frequency)
 getifreq [10.2.6](returns the getvec sampling frequency)
 getvec		(reads a (possibly resampled) sample from each input signal)
 getframe [9.0]	(reads an input frame)
 putvec		(writes a sample to each output signal)
 isigsettime	(skips to a specified time in each signal)
 isgsettime	(skips to a specified time in a specified signal group)
 setibsize [5.0](sets the default buffer size for getvec)
 setobsize [5.0](sets the default buffer size for putvec)
 newheader	(creates a new header file)
 setheader [5.0](creates or rewrites a header file given signal specifications)
 setmsheader [9.1] (creates or rewrites a header for a multi-segment record)
 wfdbgetskew [9.4](returns skew)
 wfdbsetiskew[9.8](sets skew for input signals)
 wfdbsetskew [9.4](sets skew to be written by setheader)
 wfdbgetstart [9.4](returns byte offset of sample 0 within signal file)
 wfdbsetstart [9.4](sets byte offset to be written by setheader)
 getinfo [4.0]	(reads a line of info for a record)
 putinfo [4.0]	(writes a line of info for a record)
 sampfreq	(returns the sampling frequency of the specified record)
 setsampfreq	(sets the putvec sampling frequency)
 setbasetime	(sets the base time and date)
 timstr		(converts sample intervals to time strings)
 mstimstr	(converts sample intervals to time strings with milliseconds)
 getcfreq [5.2]	(gets the counter frequency)
 setcfreq [5.2]	(sets the counter frequency)
 getbasecount [5.2] (gets the base counter value)
 setbasecount [5.2] (sets the base counter value)
 strtim		(converts time strings to sample intervals)
 datstr		(converts Julian dates to date strings)
 strdat		(converts date strings to Julian dates)
 adumuv		(converts ADC units to microvolts)
 muvadu		(converts microvolts to ADC units)
 aduphys [6.0]	(converts ADC units to physical units)
 physadu [6.0]	(converts physical units to ADC units)
 sample [10.3.0](get a sample from a given signal at a given time)
 sample_valid [10.3.0](verify that last value returned by sample was valid)
(Numbers in brackets in the list above indicate the first version of the WFDB
library that included the corresponding function.  Functions not so marked
have been included in all published versions of the WFDB library.)

These functions, also defined here, are intended only for the use of WFDB
library functions defined elsewhere:
 wfdb_sampquit  (frees memory allocated by sample())
 wfdb_sigclose 	(closes signals and resets variables)
 wfdb_osflush	(flushes output signals)

Two versions of r16() and w16() are provided here.  The default versions are
implemented as macros for efficiency.  At least one C compiler (the Microport
System V/AT `cc') is known to run out of space while attempting to compile
these macros.  To avoid this problem, define the symbol BROKEN_CC when
compiling this module, in order to obtain the alternate versions, which are
implemented as functions.

The function setbasetime() uses the C library functions localtime() and time(),
and definitions from <time.h>.  If these are not available, either find a
replacement or define the symbol NOTIME when compiling this module;  taking the
latter step will cause setbasetime() to leave the base time unchanged if it is
passed a NULL or empty argument, rather than setting it to the current time.

Beginning with version 6.1, header files are written by newheader() and
setheader() with \r\n line separators (earlier versions used \n only).  Earlier
versions of the WFDB library can read header files written by these functions,
but signal descriptions and info strings will be terminated by \r.  This change
was made so that header files can be more easily viewed and edited under
MS-DOS.

Multifrequency records (i.e., those for which not all signals are digitized at
the same frequency) are supported by version 9.0 and later versions.  Multi-
segment records (constructed by concatenating single-segment records) and null
(format 0) signals are supported by version 9.1 and later versions.

Beginning with version 10.2, there are no longer any fixed limits on the
number of signals in a record or the number of samples per signal per frame.
Older applications used the symbols WFDB_MAXSIG and WFDB_MAXSPF (which are
still defined in wfdb.h for compatibility) to determine the sizes needed for
arrays of WFDB_Siginfo structures passed into, e.g., isiginfo, and the lengths
of arrays of WFDB_Samples passed into getvec and getframe.  Newly-written
applications should use the methods illustrated immediately below instead.
*/

#if 0
/* This is sample code to show how to allocate signal information and sample
   vector arrays in an application -- it is *not* compiled into this module! */
example()
{
    int n, nsig, i, framelen;
    WFDB_Siginfo *si;
    WFDB_Sample *vector;

    /* Get the number of signals without opening any signal files. */
    n = isigopen("record", NULL, 0);
    if (n < 1) { /* no signals -- quit or try another record, etc. */ }
	
    /* Allocate WFDB_Siginfo structures before calling isigopen again. */
    si = calloc(n, sizeof(WFDB_Siginfo));
    nsig = isigopen("record", si, n);
    /* Note that nsig equals n only if all signals were readable. */

    /* Get the number of samples per frame. */
    for (i = framelen = 0; i < nsig; i++)
        framelen += si[i].spf;
    /* Allocate WFDB_Samples before calling getframe. */
    vector = calloc(framelen, sizeof(WFDB_Sample));
    getframe(vector);
}
#endif

#include "wfdblib.h"

#ifdef iAPX286
#define BROKEN_CC
#endif

#ifndef NOTIME
#include <time.h>
#endif

/* Shared local data */

/* These variables are set by readheader, and contain information about the
   signals described in the most recently opened header file.
*/
static unsigned maxhsig;	/* # of hsdata structures pointed to by hsd */
static WFDB_FILE *hheader;	/* file pointer for header file */
static struct hsdata {
    WFDB_Siginfo info;		/* info about signal from header */
    long start;			/* signal file byte offset to sample 0 */
    int skew;			/* intersignal skew (in frames) */
} **hsd;

/* Variables in this group are also set by readheader, but may be reset (by,
   e.g., setsampfreq, setbasetime, ...).  These are used by strtim, timstr,
   etc., for converting among sample intervals, counter values, elapsed times,
   and absolute times and dates; they are recorded when writing header files
   using newheader, setheader, and setmsheader.  Changing these variables has
   no effect on the data read by getframe (or getvec) or on the data written by
   putvec (although changes will affect what is written to output header files
   by setheader, etc.).  An application such as xform can use independent
   sampling frequencies and different base times or dates for input and output
   signals, but only one set of these parameters is available at any given time
   for use by the strtim, timstr, etc., conversion functions.
*/
static WFDB_Frequency ffreq;	/* frame rate (frames/second) */
static WFDB_Frequency ifreq;	/* samples/second/signal returned by getvec */
static WFDB_Frequency sfreq;	/* samples/second/signal read by getvec */
static WFDB_Frequency cfreq;	/* counter frequency (ticks/second) */
static long btime;		/* base time (milliseconds since midnight) */
static WFDB_Date bdate;		/* base date (Julian date) */
static WFDB_Time nsamples;	/* duration of signals (in samples) */
static double bcount;		/* base count (counter value at sample 0) */
static long prolog_bytes;	/* length of prolog, as told to wfdbsetstart
				   (used only by setheader, if output signal
				   file(s) are not open) */

/* The next set of variables contains information about multi-segment records.
   The first two of them ('segments' and 'in_msrec') are used primarily as
   flags to indicate if a record contains multiple segments.  Unless 'in_msrec'
   is set already, readheader sets 'segments' to the number of segments
   indicated in the header file it has most recently read (0 for a
   single-segment record).  If it reads a header file for a multi-segment
   record, readheader also sets the variables 'msbtime', 'msbdate', and
   'msnsamples'; allocates and fills 'segarray'; and sets 'segp' and 'segend'.
   Note that readheader's actions are not restricted to records opened for
   input.

   If isigopen finds that 'segments' is non-zero, it sets 'in_msrec' and then
   invokes readheader again to obtain signal information from the header file
   for the first segment, which must be a single-segment record (readheader
   refuses to open a header file for a multi-segment record if 'in_msrec' is
   set).

   When creating a header file for a multi-segment record using setmsheader,
   the variables 'msbtime', 'msbdate', and 'msnsamples' are filled in by
   setmsheader based on btime and bdate for the first segment, and on the
   sum of the 'nsamp' fields for all segments.  */
static int segments;		/* number of segments found by readheader() */
static int in_msrec;		/* current input record is: 0: a single-segment
				   record; 1: a multi-segment record */
static long msbtime;		/* base time for multi-segment record */
static WFDB_Date msbdate;	/* base date for multi-segment record */
static WFDB_Time msnsamples;	/* duration of multi-segment record */
static struct segrec {
    char recname[WFDB_MAXRNL+1];/* segment name */
    WFDB_Time nsamp;		/* number of samples in segment */
    WFDB_Time samp0;		/* sample number of first sample in segment */
} *segarray, *segp, *segend;	/* beginning, current segment, end pointers */

/* These variables relate to open input signals. */
static unsigned maxisig;	/* max number of input signals */
static unsigned maxigroup;	/* max number of input signal groups */
static unsigned nisig;		/* number of open input signals */
static unsigned nigroups;	/* number of open input signal groups */
static unsigned maxspf;		/* max allowed value for ispfmax */
static unsigned ispfmax;	/* max number of samples of any open signal
				   per input frame */
static struct isdata {		/* unique for each input signal */
    WFDB_Siginfo info;		/* input signal information */
    WFDB_Sample samp;		/* most recent sample read */
    int skew;			/* intersignal skew (in frames) */
} **isd;
static struct igdata {		/* shared by all signals in a group (file) */
    int data;			/* raw data read by r*() */
    int datb;			/* more raw data used for bit-packed formats */
    WFDB_FILE *fp;		/* file pointer for an input signal group */
    long start;			/* signal file byte offset to sample 0 */
    int bsize;			/* if non-zero, all reads from the input file
				   are in multiples of bsize bytes */
    char *buf;			/* pointer to input buffer */
    char *bp;			/* pointer to next location in buf[] */
    char *be;			/* pointer to input buffer endpoint */
    char count;			/* input counter for bit-packed signal */
    char seek;			/* flag to indicate if seeks are permitted */
    int stat;			/* signal file status flag */
} **igd;
static WFDB_Sample *tvector;	/* getvec workspace */
static WFDB_Sample *uvector;	/* isgsettime workspace */
static WFDB_Time istime;	/* time of next input sample */
static int ibsize;		/* default input buffer size */
static unsigned skewmax;	/* max skew (frames) between any 2 signals */
static WFDB_Sample *dsbuf;	/* deskewing buffer */
static int dsbi;		/* index to oldest sample in dsbuf (if < 0,
				   dsbuf does not contain valid data) */
static unsigned dsblen;		/* capacity of dsbuf, in samples */
static unsigned framelen;	/* total number of samples per frame */
static int gvmode = -1;		/* getvec mode (WFDB_HIGHRES or WFDB_LOWRES
				   once initialized) */
static int gvc;			/* getvec sample-within-frame counter */
WFDB_Sample *sbuf = NULL;	/* buffer used by sample() */
static int sample_vflag;	/* if non-zero, last value returned by sample()
				   was valid */

/* These variables relate to output signals. */
static unsigned maxosig;	/* max number of output signals */
static unsigned maxogroup;	/* max number of output signal groups */
static unsigned nosig;		/* number of open output signals */
static unsigned nogroups;	/* number of open output signal groups */
static WFDB_FILE *oheader;	/* file pointer for output header file */
static struct osdata {		/* unique for each output signal */
    WFDB_Siginfo info;		/* output signal information */
    WFDB_Sample samp;		/* most recent sample written */
    int skew;			/* skew to be written by setheader() */
} **osd;
static struct ogdata {		/* shared by all signals in a group (file) */
    int data;			/* raw data to be written by w*() */
    int datb;			/* more raw data used for bit-packed formats */
    WFDB_FILE *fp;		/* file pointer for output signal */
    long start;			/* byte offset to be written by setheader() */
    int bsize;			/* if non-zero, all writes to the output file
				   are in multiples of bsize bytes */
    char *buf;			/* pointer to output buffer */
    char *bp;			/* pointer to next location in buf[]; */
    char *be;			/* pointer to output buffer endpoint */
    char count;		/* output counter for bit-packed signal */
} **ogd;
static WFDB_Time ostime;	/* time of next output sample */
static int obsize;		/* default output buffer size */

/* Local functions (not accessible outside this file). */

/* Allocate workspace for up to n input signals. */
static int allocisig(n)
unsigned n;
{
    if (maxisig < n) {
	unsigned m = maxisig;
	struct isdata **isdnew = realloc(isd, n*sizeof(struct isdata *));

	if (isdnew == NULL) {
	    wfdb_error("init: too many (%d) input signals\n", n);
	    return (-1);
	}
	isd = isdnew;
	while (m < n) {
	    if ((isd[m] = calloc(1, sizeof(struct isdata))) == NULL) {
		wfdb_error("init: too many (%d) input signals\n", n);
		while (--m > maxisig)
		    free(isd[m]);
		return (-1);
	    }
	    m++;
	}
	maxisig = n;
    }
    return (maxisig);
}

/* Allocate workspace for up to n input signal groups. */
static int allocigroup(n)
unsigned n;
{
    if (maxigroup < n) {
	unsigned m = maxigroup;
	struct igdata **igdnew = realloc(igd, n*sizeof(struct igdata *));

	if (igdnew == NULL) {
	    wfdb_error("init: too many (%d) input signal groups\n", n);
	    return (-1);
	}
	igd = igdnew;
	while (m < n) {
	    if ((igd[m] = calloc(1, sizeof(struct igdata))) == NULL) {
		wfdb_error("init: too many (%d) input signal groups\n", n);
		while (--m > maxigroup)
		    free(igd[m]);
		return (-1);
	    }
	    m++;
	}
	maxigroup = n;
    }
    return (maxigroup);
}

/* Allocate workspace for up to n output signals. */
static int allocosig(n)
unsigned n;
{
    if (maxosig < n) {
	unsigned m = maxosig;
	struct osdata **osdnew = realloc(osd, n*sizeof(struct osdata *));

	if (osdnew == NULL) {
	    wfdb_error("init: too many (%d) output signals\n", n);
	    return (-1);
	}
	osd = osdnew;
	while (m < n) {
	    if ((osd[m] = calloc(1, sizeof(struct osdata))) == NULL) {
		wfdb_error("init: too many (%d) output signals\n", n);
		while (--m > maxosig)
		    free(osd[m]);
		return (-1);
	    }
	    m++;
	}
	maxosig = n;
    }
    return (maxosig);
}

/* Allocate workspace for up to n output signal groups. */
static int allocogroup(n)
unsigned n;
{
    if (maxogroup < n) {
	unsigned m = maxogroup;
	struct ogdata **ogdnew = realloc(ogd, n*sizeof(struct ogdata *));

	if (ogdnew == NULL) {
	    wfdb_error("init: too many (%d) output signal groups\n", n);
	    return (-1);
	}
	ogd = ogdnew;
	while (m < n) {
	    if ((ogd[m] = calloc(1, sizeof(struct ogdata))) == NULL) {
		wfdb_error("init: too many (%d) output signal groups\n", n);
		while (--m > maxogroup)
		    free(ogd[m]);
		return (-1);
	    }
	    m++;
	}
	maxogroup = n;
    }
    return (maxogroup);
}

static int isfmt(f)
int f;
{
    int i;
    static int fmt_list[WFDB_NFMTS] = WFDB_FMT_LIST;

    for (i = 0; i < WFDB_NFMTS; i++)
	if (f == fmt_list[i]) return (1);
    return (0);
}

static int copysi(to, from)
WFDB_Siginfo *to, *from;
{
    if (to == NULL || from == NULL) return (0);
    *to = *from;
    /* The next line works around an optimizer bug in gcc (version 2.96, maybe
       others). */
    to->fname = to->desc = to->units = NULL;
    if (from->fname) {
        to->fname = (char *)malloc((size_t)strlen(from->fname)+1);
	if (to->fname == NULL) return (-1);
	(void)strcpy(to->fname, from->fname);
    }
    if (from->desc) {
        to->desc = (char *)malloc((size_t)strlen(from->desc)+1);
	if (to->desc == NULL) {
	    (void)free(to->fname);
	    return (-1);
	}
	(void)strcpy(to->desc, from->desc);
    }
    if (from->units) {
        to->units = (char *)malloc((size_t)strlen(from->units)+1);
	if (to->units == NULL) {
	    (void)free(to->desc);
	    (void)free(to->fname);
	    return (-1);
	}
	(void)strcpy(to->units, from->units);
    }
    return (1);
}

static int readheader(record)
char *record;
{
    char linebuf[256], *p, *q;
    WFDB_Frequency f;
    WFDB_Signal s;
    WFDB_Time ns;
    unsigned int i, nsig;
    static char sep[] = " \t\n\r";

    /* If another input header file was opened, close it. */
    if (hheader) (void)wfdb_fclose(hheader);

    /* Try to open the header file. */
    if ((hheader = wfdb_open("hea", record, WFDB_READ)) == NULL) {
	wfdb_error("init: can't open header for record %s\n", record);
	return (-1);
    }

    /* Get the first token (the record name) from the first non-empty,
       non-comment line. */
    do {
	if (wfdb_fgets(linebuf, 256, hheader) == NULL) {
	    wfdb_error("init: can't find record name in record %s header\n",
		     record);
	    return (-2);
	}
    } while ((p = strtok(linebuf, sep)) == NULL || *p == '#');
    for (q = p+1; *q && *q != '/'; q++)
	;
    if (*q == '/') {
	if (in_msrec) {
	    wfdb_error(
	  "init: record %s cannot be nested in another multi-segment record\n",
		     record);
	    return (-2);
	}
	segments = atoi(q+1);
	*q = '\0';
    }

    /* For local files, be sure that the name (p) within the header file
       matches the name (record) provided as an argument to this function --
       if not, the header file may have been renamed in error or its contents
       may be corrupted.  The requirement for a match is waived for remote
       files since the user may not be able to make any corrections to them. */
    if (hheader->type == WFDB_LOCAL &&
	hheader->fp != stdin && strcmp(p, record) != 0) {
	/* If there is a mismatch, check to see if the record argument includes
	   a directory separator (whether valid or not for this OS);  if so,
	   compare only the final portion of the argument against the name in
	   the header file. */
	char *r, *s;

	for (r = record, s = r + strlen(r) - 1; r != s; s--)
	    if (*s == '/' || *s == '\\' || *s == ':')
		break;

	if (r > s || strcmp(p, s+1) != 0) {
	    wfdb_error("init: record name in record %s header is incorrect\n",
		       record);
	    return (-2);
	}
    }

    /* Identify which type of header file is being read by trying to get
       another token from the line which contains the record name.  (Old-style
       headers have only one token on the first line, but new-style headers
       have two or more.) */
    if ((p = strtok((char *)NULL, sep)) == NULL) {
	/* The file appears to be an old-style header file. */
	wfdb_error("init: obsolete format in record %s header\n", record);
	return (-2);
    }

    /* The file appears to be a new-style header file.  The second token
       specifies the number of signals. */
    nsig = (unsigned)atoi(p);

    /* Determine the frame rate, if present and not set already. */
    if (p = strtok((char *)NULL, sep)) {
	if ((f = (WFDB_Frequency)atof(p)) <= (WFDB_Frequency)0.) {
	    wfdb_error(
		 "init: sampling frequency in record %s header is incorrect\n",
		 record);
	    return (-2);
	}
	if (ffreq > (WFDB_Frequency)0. && f != ffreq) {
	    wfdb_error("warning (init):\n");
	    wfdb_error(" record %s sampling frequency differs", record);
	    wfdb_error(" from that of previously opened record\n");
	}
	else
	    ffreq = f;
    }
    else if (ffreq == (WFDB_Frequency)0.)
	ffreq = WFDB_DEFFREQ;

    /* Set the sampling rate to the frame rate for now.  This may be
       changed later by isigopen or by setgvmode, if this is a multi-
       frequency record and WFDB_HIGHRES mode is in effect. */
    sfreq = ffreq;

    /* Determine the counter frequency and the base counter value. */
    cfreq = bcount = 0.0;
    if (p) {
	for ( ; *p && *p != '/'; p++)
	    ;
	if (*p == '/') {
	    cfreq = atof(++p);
	    for ( ; *p && *p != '('; p++)
		;
	    if (*p == '(')
		bcount = atof(++p);
	}
    }
    if (cfreq <= 0.0) cfreq = ffreq;

    /* Determine the number of samples per signal, if present and not
       set already. */
    if (p = strtok((char *)NULL, sep)) {
	if ((ns = (WFDB_Time)atol(p)) < 0L) {
	    wfdb_error(
		"init: number of samples in record %s header is incorrect\n",
		record);
	    return (-2);
	}
	if (nsamples == (WFDB_Time)0L)
	    nsamples = ns;
	else if (ns > (WFDB_Time)0L && ns != nsamples && !in_msrec) {
	    wfdb_error("warning (init):\n");
	    wfdb_error(" record %s duration differs", record);
	    wfdb_error(" from that of previously opened record\n");
	    /* nsamples must match the shortest record duration. */
	    if (nsamples > ns)
		nsamples = ns;
	}
    }
    else
	ns = (WFDB_Time)0L;

    /* Determine the base time and date, if present and not set already. */
    if ((p = strtok((char *)NULL,"\n\r")) != NULL &&
	btime == 0L && setbasetime(p) < 0)
	return (-2);	/* error message will come from setbasetime */

    /* Special processing for master header of a multi-segment record. */
    if (segments && !in_msrec) {
	msbtime = btime;
	msbdate = bdate;
	msnsamples = nsamples;
	/* Read the names and lengths of the segment records. */
	segarray = (struct segrec *)calloc(segments, sizeof(struct segrec));
	if (segarray == (struct segrec *)NULL) {
	    wfdb_error("init: insufficient memory\n");
	    segments = 0;
	    return (-2);
	}
	segp = segarray;
	for (i = 0, ns = (WFDB_Time)0L; i < segments; i++, segp++) {
	    /* Get next segment spec, skip empty lines and comments. */
	    do {
		if (wfdb_fgets(linebuf, 256, hheader) == NULL) {
		    wfdb_error(
			"init: unexpected EOF in header file for record %s\n",
			record);
		    (void)free(segarray);
		    segarray = (struct segrec *)NULL;
		    segments = 0;
		    return (-2);
		}
	    } while ((p = strtok(linebuf, sep)) == NULL || *p == '#');
	    if (strlen(p) > WFDB_MAXRNL) {
		wfdb_error(
		    "init: `%s' is too long for a segment name in record %s\n",
		    p, record);
		(void)free(segarray);
		segarray = (struct segrec *)NULL;
		segments = 0;
		return (-2);
	    }
	    (void)strcpy(segp->recname, p);
	    if ((p = strtok((char *)NULL, sep)) == NULL ||
		(segp->nsamp = (WFDB_Time)atol(p)) <= 0L) {
		wfdb_error(
		"init: length must be specified for segment %s in record %s\n",
		           segp->recname, record);
		(void)free(segarray);
		segarray = (struct segrec *)NULL;
		segments = 0;
		return (-2);
	    }
	    segp->samp0 = ns;
	    ns += segp->nsamp;
	}
	segend = --segp;
	segp = segarray;
	if (msnsamples == 0L)
	    msnsamples = ns;
	else if (ns != msnsamples) {
	    wfdb_error("warning (init): sum of segment lengths (%ld)\n", ns);
	    wfdb_error("   does not match stated record length (%ld)\n",
		       msnsamples);
	}
	return (0);
    }

    /* Allocate workspace. */
    if (maxhsig < nsig) {
	unsigned m = maxhsig;
	struct hsdata **hsdnew = realloc(hsd, nsig*sizeof(struct hsdata *));

	if (hsdnew == NULL) {
	    wfdb_error("init: too many (%d) signals in header file\n", nsig);
	    return (-2);
	}
	hsd = hsdnew;
	while (m < nsig) {
	    if ((hsd[m] = calloc(1, sizeof(struct hsdata))) == NULL) {
		wfdb_error("init: too many (%d) signals in header file\n",
			   nsig);
		while (--m > maxhsig)
		    free(hsd[m]);
		return (-2);
	    }
	    m++;
	}
	maxhsig = nsig;
    }

    /* Now get information for each signal. */
    for (s = 0; s < nsig; s++) {
	struct hsdata *hp, *hs;
	int nobaseline;

	hs = hsd[s];
	if (s) hp = hsd[s-1];
	/* Get the first token (the signal file name) from the next
	   non-empty, non-comment line. */
	do {
	    if (wfdb_fgets(linebuf, 256, hheader) == NULL) {
		wfdb_error(
			"init: unexpected EOF in header file for record %s\n",
			record);
		return (-2);
	    }
	} while ((p = strtok(linebuf, sep)) == NULL || *p == '#');

	/* Determine the signal group number.  The group number for signal
	   0 is zero.  For subsequent signals, if the file name does not
	   match that of the previous signal, the group number is one
	   greater than that of the previous signal. */
	if (s == 0 || strcmp(p, hp->info.fname)) {
		hs->info.group = (s == 0) ? 0 : hp->info.group + 1;
		if ((hs->info.fname =(char *)malloc((unsigned)(strlen(p)+1)))
		     == NULL) {
		    wfdb_error("init: insufficient memory\n");
		    return (-2);
		}
		(void)strcpy(hs->info.fname, p);
	}
	/* If the file names of the current and previous signals match,
	   they are assigned the same group number and share a copy of the
	   file name.  All signals associated with a given file must be
	   listed together in the header in order to be identified as
	   belonging to the same group;  readheader does not check that
	   this has been done. */
	else {
	    hs->info.group = hp->info.group;
	    if ((hs->info.fname = (char *)malloc(strlen(hp->info.fname)+1))
		     == NULL) {
		    wfdb_error("init: insufficient memory\n");
		    return (-2);
		}
		(void)strcpy(hs->info.fname, hp->info.fname);
	}

	/* Determine the signal format. */
	if ((p = strtok((char *)NULL, sep)) == NULL ||
	    !isfmt(hs->info.fmt = atoi(p))) {
	    wfdb_error("init: illegal format for signal %d, record %s\n",
		       s, record);
	    return (-2);
	}
	hs->info.spf = 1;
	hs->skew = 0;
	hs->start = 0L;
	while (*(++p)) {
	    if (*p == 'x' && *(++p))
		if ((hs->info.spf = atoi(p)) < 1) hs->info.spf = 1;
	    if (*p == ':' && *(++p))
		if ((hs->skew = atoi(p)) < 0) hs->skew = 0;
	    if (*p == '+' && *(++p))
		if ((hs->start = atol(p)) < 0L) hs->start = 0L;
	}
	/* The resolution for deskewing is one frame.  The skew in samples
	   (given in the header) is converted to skew in frames here. */
	hs->skew = (int)(((double)hs->skew)/hs->info.spf + 0.5);

	/* Determine the gain in ADC units per physical unit.  This number
	   may be zero or missing;  if so, the signal is uncalibrated. */
	if (p = strtok((char *)NULL, sep))
	    hs->info.gain = (WFDB_Gain)atof(p);
	else
	    hs->info.gain = (WFDB_Gain)0.;

	/* Determine the baseline if specified, and the physical units
	   (assumed to be millivolts unless otherwise specified). */
	nobaseline = 1;
	if (p) {
	    for ( ; *p && *p != '(' && *p != '/'; p++)
		;
	    if (*p == '(') {
		hs->info.baseline = atoi(++p);
		nobaseline = 0;
	    }
	    while (*p)
		if (*p++ == '/' && *p)
		    break;
	}
	if (p && *p) {
	    if ((hs->info.units=(char *)malloc(WFDB_MAXUSL+1)) == NULL) {
		wfdb_error("init: insufficient memory\n");
		return (-2);
	    }
	    (void)strncpy(hs->info.units, p, WFDB_MAXUSL);
	}
	else
	    hs->info.units = NULL;

	/* Determine the ADC resolution in bits.  If this number is
	   missing and cannot be inferred from the format, the default
	   value (from wfdb.h) is filled in. */
	if (p = strtok((char *)NULL, sep))
	    i = (unsigned)atoi(p);
	else switch (hs->info.fmt) {
	  case 80: i = 8; break;
	  case 160: i = 16; break;
	  case 212: i = 12; break;
	  case 310: i = 10; break;
	  case 311: i = 10; break;
	  default: i = WFDB_DEFRES; break;
	}
	hs->info.adcres = i;

	/* Determine the ADC zero (assumed to be zero if missing). */
	hs->info.adczero = (p = strtok((char *)NULL, sep)) ? atoi(p) : 0;
	    
	/* Set the baseline to adczero if no baseline field was found. */
	if (nobaseline) hs->info.baseline = hs->info.adczero;

	/* Determine the initial value (assumed to be equal to the ADC 
	   zero if missing). */
	hs->info.initval = (p = strtok((char *)NULL, sep)) ?
	    atoi(p) : hs->info.adczero;

	/* Determine the checksum (assumed to be zero if missing). */
	if (p = strtok((char *)NULL, sep)) {
	    hs->info.cksum = atoi(p);
	    hs->info.nsamp = ns;
	}
	else {
	    hs->info.cksum = 0;
	    hs->info.nsamp = (WFDB_Time)0L;
	}

	/* Determine the block size (assumed to be zero if missing). */
	hs->info.bsize = (p = strtok((char *)NULL, sep)) ? atoi(p) : 0;

	/* Check that formats and block sizes match for signals belonging
	   to the same group. */
	if (s && hs->info.group == hp->info.group &&
	    (hs->info.fmt != hp->info.fmt ||
	     hs->info.bsize != hp->info.bsize)) {
	    wfdb_error("init: error in specification of signal %d or %d\n",
		       s-1, s);
	    return (-2);
	}
	    
	/* Get the signal description.  If missing, a description of
	   the form "record xx, signal n" is filled in. */
	if ((hs->info.desc = (char *)malloc(WFDB_MAXDSL+1)) == NULL) {
	    wfdb_error("init: insufficient memory\n");
	    return (-2);
	}
	if (p = strtok((char *)NULL, "\n\r"))
	    (void)strncpy(hs->info.desc, p, WFDB_MAXDSL);
	else
	    (void)sprintf(hs->info.desc,
			  "record %s, signal %d", record, s);
    }
    return (s);			/* return number of available signals */
}

static void hsdfree()
{
    struct hsdata *hs;

    if (hsd) {
	while (maxhsig)
	    if (hs = hsd[--maxhsig]) {
		if (hs->info.fname) (void)free(hs->info.fname);
		if (hs->info.units) (void)free(hs->info.units);
		if (hs->info.desc)  (void)free(hs->info.desc);
		(void)free(hs);
	    }
	(void)free(hsd);
	hsd = NULL;
    }
    maxhsig = 0;
}
		
static void isigclose()
{
    struct isdata *is;
    struct igdata *ig;

    if (nisig == 0) return;
    if (sbuf && !in_msrec) {
	(void)free(sbuf);
	sbuf = NULL;
	sample_vflag = 0;
    }
    if (isd) {
	while (nisig)
	    if (is = isd[--nisig]) {
		if (is->info.fname) (void)free(is->info.fname);
		if (is->info.units) (void)free(is->info.units);
		if (is->info.desc)  (void)free(is->info.desc);
		(void)free(is);
	    }
	(void)free(isd);
	isd = NULL;
    }
    else
	nisig = 0;
    maxisig = 0;

    if (igd) {
	while (nigroups)
	    if (ig = igd[--nigroups]) {
		if (ig->fp) (void)wfdb_fclose(ig->fp);
		if (ig->buf) (void)free(ig->buf);
		(void)free(ig);
	    }
	(void)free(igd);
	igd = NULL;
    }
    else
	nigroups = 0;
    maxigroup = 0;

    istime = 0L;
    gvc = ispfmax = 1;
    if (hheader) {
	(void)wfdb_fclose(hheader);
	hheader = NULL;
    }
    if (nosig == 0 && maxhsig != 0)
	hsdfree();
}

static void osigclose()
{
    struct osdata *os;
    struct ogdata *og;

    if (nosig == 0) return;
    if (osd) {
	while (nosig)
	    if (os = osd[--nosig]) {
		if (os->info.fname) (void)free(os->info.fname);
		if (os->info.units) (void)free(os->info.units);
		if (os->info.desc)  (void)free(os->info.desc);
		(void)free(os);
	    }
	(void)free(osd);
	osd = NULL;
    }
    else
	nosig = 0;
    maxosig = 0;

    if (ogd) {
	while (nogroups)
	    if (og = ogd[--nogroups]) {
		if (og->fp) {
		    /* If a block size has been defined, null-pad the buffer */
		    if (og->bsize)
			while (og->bp != og->be)
			    *(og->bp++) = '\0';
		    /* Flush the last block unless it's empty. */
		    if (og->bp != og->buf)
			(void)wfdb_fwrite(og->buf, 1, og->bp-og->buf, og->fp);
		    /* Close file (except stdout, which is closed on exit). */
		    if (og->fp->fp != stdout) {
			(void)wfdb_fclose(og->fp);
			og->fp = NULL;
		    }
		}
		if (og->buf) (void)free(og->buf);
		(void)free(og);
	    }
	(void)free(ogd);
	ogd = NULL;
    }
    else
	nogroups = 0;
    maxogroup = 0;

    ostime = 0L;
    if (oheader) {
	(void)wfdb_fclose(oheader);
	oheader = NULL;
    }
    if (nisig == 0 && maxhsig != 0)
	hsdfree();
}

/* Low-level I/O routines.  The input routines each get a single argument (the
signal group pointer).  The output routines get two arguments (the value to be
written and the signal group pointer). */

static int _l;		    /* macro temporary storage for low byte of word */
static int _n;		    /* macro temporary storage for byte count */

#define r8(G)	((G->bp < G->be) ? *(G->bp++) : \
		  ((_n = (G->bsize > 0) ? G->bsize : ibsize), \
		   (G->stat = _n = wfdb_fread(G->buf, 1, _n, G->fp)), \
		   (G->be = (G->bp = G->buf) + _n),\
		  *(G->bp++)))

#define w8(V,G)	(((*(G->bp++) = (char)V)), \
		  (_l = (G->bp != G->be) ? 0 : \
		   ((_n = (G->bsize > 0) ? G->bsize : obsize), \
		    wfdb_fwrite((G->bp = G->buf), 1, _n, G->fp))))

/* If a short integer is not 16 bits, it may be necessary to redefine r16() and
r61() in order to obtain proper sign extension. */

#ifndef BROKEN_CC
#define r16(G)	    (_l = r8(G), ((int)((short)((r8(G) << 8) | (_l & 0xff)))))
#define w16(V,G)    (w8((V), (G)), w8(((V) >> 8), (G)))
#define r61(G)      (_l = r8(G), ((int)((short)((r8(G) & 0xff) | (_l << 8)))))
#define w61(V,G)    (w8(((V) >> 8), (G)), w8((V), (G)))
#else

static int r16(g)
struct igdata *g;
{
    int l, h;

    l = r8(g);
    h = r8(g);
    return ((int)((short)((h << 8) | (l & 0xff))));
}

static void w16(v, g)
WFDB_Sample v;
struct ogdata *g;
{
    w8(v, g);
    w8((v >> 8), g);
}

static int r61(g)
struct igdata *g;
{
    int l, h;

    h = r8(g);
    l = r8(g);
    return ((int)((short)((h << 8) | (l & 0xff))));
}

static void w61(v, g)
WFDB_Sample v;
struct ogdata *g;
{
    w8((v >> 8), g);
    w8(v, g);
}
#endif

#define r80(G)		((r8(G) & 0xff) - (1 << 7))
#define w80(V, G)	(w8(((V) & 0xff) + (1 << 7), G))

#define r160(G)		((r16(G) & 0xffff) - (1 << 15))
#define w160(V, G)	(w16(((V) & 0xffff) + (1 << 15), G))

static int r212(g)  /* read and return the next sample from a format 212    */
struct igdata *g;   /* signal file (2 12-bit samples bit-packed in 3 bytes) */
{
    int v;

    /* Obtain the next 12-bit value right-justified in v. */
    switch (g->count++) {
      case 0:	v = g->data = r16(g); break;
      case 1:
      default:	g->count = 0;
	        v = ((g->data >> 4) & 0xf00) | (r8(g) & 0xff); break;
    }
    /* Sign-extend from the twelfth bit. */
    if (v & 0x800) v |= ~(0xfff);
    else v &= 0xfff;
    return (v);
}

static void w212(v, g)	/* write the next sample to a format 212 signal file */
WFDB_Sample v;
struct ogdata *g;
{
    /* Samples are buffered here and written in pairs, as three bytes. */
    switch (g->count++) {
      case 0:	g->data = v & 0xfff; break;
      case 1:	g->count = 0;
	  g->data |= (v << 4) & 0xf000;
	  w16(g->data, g);
	  w8(v, g);
		break;
    }
}

static int r310(g)  /* read and return the next sample from a format 310    */
struct igdata *g;   /* signal file (3 10-bit samples bit-packed in 4 bytes) */
{
    int v;

    /* Obtain the next 10-bit value right-justified in v. */
    switch (g->count++) {
      case 0:	v = (g->data = r16(g)) >> 1; break;
      case 1:	v = (g->datb = r16(g)) >> 1; break;
      case 2:
      default:	g->count = 0;
		v = ((g->data & 0xf800) >> 11) | ((g->datb & 0xf800) >> 6);
		break;
    }
    /* Sign-extend from the tenth bit. */
    if (v & 0x200) v |= ~(0x3ff);
    else v &= 0x3ff;
    return (v);
}

static void w310(v, g)	/* write the next sample to a format 310 signal file */
WFDB_Sample v;
struct ogdata *g;
{
    /* Samples are buffered here and written in groups of three, as two
       left-justified 15-bit words. */
    switch (g->count++) {
      case 0:	g->data = (v << 1) & 0x7fe; break;
      case 1:	g->datb = (v << 1) & 0x7fe; break;
      case 2:
      default:	g->count = 0;
	        g->data |= (v << 11); w16(g->data, g);
	        g->datb |= ((v <<  6) & ~0x7fe); w16(g->datb, g);
		break;
    }
}

/* Note that formats 310 and 311 differ in the layout of the bit-packed data */

static int r311(g)  /* read and return the next sample from a format 311    */
struct igdata *g;   /* signal file (3 10-bit samples bit-packed in 4 bytes) */
{
    int v;

    /* Obtain the next 10-bit value right-justified in v. */
    switch (g->count++) {
      case 0:	v = (g->data = r16(g)); break;
      case 1:	g->datb = r16(g);
	        v = ((g->data & 0xfc00) >> 10) | ((g->datb & 0xf) << 6);
		break;
      case 2:
      default:	g->count = 0;
		v = g->datb >> 4; break;
    }
    /* Sign-extend from the tenth bit. */
    if (v & 0x200) v |= ~(0x3ff);
    else v &= 0x3ff;
    return (v);
}

static void w311(v, g)	/* write the next sample to a format 311 signal file */
WFDB_Sample v;
struct ogdata *g;
{
    /* Samples are buffered here and written in groups of three, bit-packed
       into the 30 low bits of a 32-bit word. */
    switch (g->count++) {
      case 0:	g->data = v & 0x3ff; break;
      case 1:	g->data |= (v << 10); w16(g->data, g);
	        g->datb = (v >> 6) & 0xf; break;
      case 2:
      default:	g->count = 0;
	        g->datb |= (v << 4); g->datb &= 0x3fff; w16(g->datb, g);
		break;
    }
}

static int isgsetframe(g, t)
WFDB_Group g;
WFDB_Time t;
{
    int i, trem = 0;
    long nb, tt;
    struct igdata *ig;
    WFDB_Signal s;
    unsigned int b, d = 1, n, nn;


    /* Find the first signal that belongs to group g. */
    for (s = 0; s < nisig && g != isd[s]->info.group; s++)
	;
    if (s == nisig) {
	wfdb_error("isgsettime: incorrect signal group number %d\n", g);
	return (-2);
    }

    /* Mark the contents of the deskewing buffer (if any) as invalid. */
    dsbi = -1;

    /* If the current record contains multiple segments, locate the segment
       containing the desired sample. */
    if (in_msrec) {
	struct segrec *tseg = segp;

	if (t >= msnsamples) {
	    wfdb_error("isigsettime: improper seek on signal group %d\n", g);
	    return (-1);
	}
	while (t < tseg->samp0)
	    tseg--;
	while (t >= tseg->samp0 + tseg->nsamp && tseg < segend)
	    tseg++;
	if (segp != tseg) {
	    segp = tseg;
	    if (isigopen(segp->recname, NULL, (int)nisig) != nisig) {
	        wfdb_error("isigsettime: can't open segment %s\n",
			   segp->recname);
		return (-1);
	    }
	}
	t -= segp->samp0;
    }

    ig = igd[g];
    /* Determine the number of samples per frame for signals in the group. */
    for (n = nn = 0; s+n < nisig && isd[s+n]->info.group == g; n++)
	nn += isd[s+n]->info.spf;
    /* Determine the number of bytes per sample interval in the file. */
    switch (isd[s]->info.fmt) {
      case 0:
	if (t < nsamples) {
	    if (s == 0) istime = (in_msrec) ? t + segp->samp0 : t;
	    isd[s]->info.nsamp = nsamples - t;
	    return (ig->stat = 1);
	}
	else {
	    if (s == 0) istime = (in_msrec) ? msnsamples : nsamples;
	    isd[s]->info.nsamp = 0L;
	    return (-1);
	}
      case 8:
      case 80:
      default: b = nn; break;
      case 16:
      case 61:
      case 160: b = 2*nn; break;
      case 212:
	/* Reset the input counter. */
	ig->count = 0;
	/* If the desired sample does not lie on a byte boundary, seek to
	   the previous sample and then read ahead. */
	if ((nn & 1) && (t & 1)) {
	    if (in_msrec)
		t += segp->samp0;	/* restore absolute time */
	    if (i = isgsetframe(g, t - 1))
		return (i);
	    for (i = 0; i < nn; i++)
		(void)r212(ig);
	    istime++;
	    return (0);
	}
	b = 3*nn; d = 2; break;
      case 310:
	/* Reset the input counter. */
	ig->count = 0;
	/* If the desired sample does not lie on a byte boundary, seek to
	   the closest previous sample that does, then read ahead. */
	if ((nn % 3) && (trem = (t % 3))) {
	    if (in_msrec)
		t += segp->samp0;	/* restore absolute time */
	    if (i = isgsetframe(g, t - trem))
		return (i);
	    for (i = nn*trem; i > 0; i--)
		(void)r310(ig);
	    istime += trem;
	    return (0);
	}		  
	b = 4*nn; d = 3; break;
      case 311:
	/* Reset the input counter. */
	ig->count = 0;
	/* If the desired sample does not lie on a byte boundary, seek to
	   the closest previous sample that does, then read ahead. */
	if ((nn % 3) && (trem = (t % 3))) {
	    if (in_msrec)
		t += segp->samp0;	/* restore absolute time */
	    if (i = isgsetframe(g, t - trem))
		return (i);
	    for (i = nn*trem; i > 0; i--)
		(void)r311(ig);
	    istime += trem;
	    return (0);
	}		  
	b = 4*nn; d = 3; break;
    }

    /* Seek to the beginning of the block which contains the desired sample.
       For normal files, use fseek() to do so. */
    if (ig->seek) {
	tt = t*b;
	nb = tt/d + ig->start;
	if ((i = ig->bsize) == 0) i = ibsize;
	/* Seek to a position such that the next block read will contain the
	   desired sample. */
	tt = nb/i;
	if (wfdb_fseek(ig->fp, tt*i, 0)) {
	    wfdb_error("isigsettime: improper seek on signal group %d\n", g);
	    return (-1);
	}
	nb %= i;
    }
    /* For special files, rewind if necessary and then read ahead. */
    else {
	long t0, t1;

	/* Get the time of the earliest buffered sample ... */
	t0 = istime - (ig->bp - ig->buf)/b;
	/* ... and that of the earliest unread sample. */
	t1 = t0 + (ig->be - ig->buf)/b;
	/* There are three possibilities:  either the desired sample has been
	   read and has passed out of the buffer, requiring a rewind ... */
	if (t < t0) {
	    if (wfdb_fseek(ig->fp, 0L, 0)) {
		wfdb_error("isigsettime: improper seek on signal group %d\n",
			   g);
		return (-1);
	    }
	    tt = t*b;
	    nb = tt/d + ig->start;
	}
	/* ... or it is in the buffer already ... */
	else if (t < t1) {
	    tt = (t - t0)*b;
	    ig->bp = ig->buf + tt/d;
	    return (0);
	}
	/* ... or it has not yet been read. */
	else {
	    tt = (t - t1) * b;
	    nb = tt/d;
	}
	while (nb > ig->bsize && !wfdb_feof(ig->fp))
	    nb -= wfdb_fread(ig->buf, 1, ig->bsize, ig->fp);
    }

    /* Reset the block pointer to indicate nothing has been read in the
       current block. */
    ig->bp = ig->be;
    ig->stat = 1;
    /* Read any bytes in the current block that precede the desired sample. */
    while (nb-- > 0 && ig->stat > 0)
	i = r8(ig);
    if (ig->stat <= 0) return (-1);

    /* Reset the getvec sample-within-frame counter. */
    gvc = ispfmax;

    /* Reset the time (if signal 0 belongs to the group) and disable checksum
       testing (by setting the number of samples remaining to 0). */
    if (s == 0) istime = in_msrec ? t + segp->samp0 : t;
    while (n-- != 0)
	isd[s+n]->info.nsamp = (WFDB_Time)0L;
    return (0);
}

static int getskewedframe(vector)
WFDB_Sample *vector;
{
    int c, stat;
    struct isdata *is;
    struct igdata *ig;
    WFDB_Group g;
    WFDB_Signal s;

    if ((stat = (int)nisig) == 0) return (0);
    if (istime == 0L) {
	for (s = 0; s < nisig; s++)
	    isd[s]->samp = isd[s]->info.initval;
	for (g = nigroups; g; ) {
	    /* Go through groups in reverse order since seeking on group 0
	       should always be done last. */
	    if (--g == 0 || igd[g]->start > 0L)
		(void)isgsetframe(g, 0L);
	}
    }

    for (s = 0; s < nisig; s++) {
	is = isd[s];
	ig = igd[is->info.group];
	for (c = 0; c < is->info.spf; c++, vector++) {
	    switch (is->info.fmt) {
	      case 0:	/* null signal: return adczero */
		*vector = is->info.adczero;
		if (is->info.nsamp == 0) ig->stat = -1;
		break;
	      case 8:	/* 8-bit first differences */
	      default:
		*vector = is->samp += r8(ig); break;
	      case 16:	/* 16-bit amplitudes */
		*vector = r16(ig); break;
	      case 61:	/* 16-bit amplitudes, bytes swapped */
		*vector = r61(ig); break;
	      case 80:	/* 8-bit offset binary amplitudes */
		*vector = r80(ig); break;
	      case 160:	/* 16-bit offset binary amplitudes */
		*vector = r160(ig); break;
	      case 212:	/* 2 12-bit amplitudes bit-packed in 3 bytes */
		*vector = r212(ig); break;
	      case 310:	/* 3 10-bit amplitudes bit-packed in 4 bytes */
		*vector = r310(ig); break;
	      case 311:	/* 3 10-bit amplitudes bit-packed in 4 bytes */
		*vector = r311(ig); break;
	    }
	    if (ig->stat <= 0) {
		/* End of file -- reset input counter. */
		ig->count = 0;
		if (is->info.nsamp > (WFDB_Time)0L) {
		    wfdb_error("getvec: unexpected EOF in signal %d\n", s);
		    stat = -3;
		}
		else if (in_msrec && segp < segend) {
		    segp++;
		    if (isigopen(segp->recname, NULL, (int)nisig) < nisig) {
			wfdb_error("getvec: error opening segment %s\n",
				   segp->recname);
			stat = -3;
		    }
		    else {
			istime = segp->samp0;
			return (getskewedframe(vector));
		    }
		}
		else
		    stat = -1;
		if (is->info.nsamp > (WFDB_Time)0L) {
		    wfdb_error("getvec: unexpected EOF in signal %d\n", s);
		    stat = -3;
		}
		else
		    stat = -1;
	    }
	    else
		is->info.cksum -= *vector;
	}
	if (--is->info.nsamp == (WFDB_Time)0L &&
	    (is->info.cksum & 0xffff) &&
	    is->info.fmt != 0) {
	    wfdb_error("getvec: checksum error in signal %d\n", s);
	    stat = -4;
	}
    }
    return (stat);
}

static int rgetvec(vector)
WFDB_Sample *vector;
{
    WFDB_Sample *tp;
    WFDB_Signal s;
    static int stat;

    if (ispfmax < 2)	/* all signals at the same frequency */
	return (getframe(vector));

    if (gvmode != WFDB_HIGHRES) {/* return one sample per frame, decimating
				   (by averaging) if necessary */
	unsigned c;
	long v;

	stat = getframe(tvector);
	for (s = 0, tp = tvector; s < nisig; s++) {
	    int sf = isd[s]->info.spf;

	    for (c = v = 0; c < sf; c++)
		v += *tp++;
	    *vector++ = v/sf;
	}
    }
    else {			/* return ispfmax samples per frame, using
				   zero-order interpolation if necessary */
	if (gvc >= ispfmax) {
	    stat = getframe(tvector);
	    gvc = 0;
	}
	for (s = 0, tp = tvector; s < nisig; s++) {
	    int sf = isd[s]->info.spf;

	    *vector++ = tp[(sf*gvc)/ispfmax];
	    tp += sf;
	}
	gvc++;
    }
    return (stat);
}

/* WFDB library functions. */

FINT isigopen(record, siarray, nsig)
char *record;
WFDB_Siginfo *siarray;
int nsig;
{
    int navail, ngroups, nn;
    struct hsdata *hs;
    struct isdata *is;
    struct igdata *ig;
    WFDB_Signal s, si, sj;
    WFDB_Group g;

    /* Close previously opened input signals unless otherwise requested. */
    if (*record == '+') record++;
    else isigclose();

    /* Save the current record name. */
    if (!in_msrec) wfdb_setirec(record);

    /* Read the header and determine how many signals are available. */
    if ((navail = readheader(record)) <= 0) {
	if (navail == 0 && segments) {	/* this is a multi-segment record */
	    in_msrec = 1;
	    /* Open the first segment to get signal information. */
	    if ((navail = readheader(segp->recname)) >= 0) {
		if (msbtime == 0L) msbtime = btime;
		if (msbdate == (WFDB_Date)0) msbdate = bdate;
	    }
	}
	if (navail == 0 && nsig)
	    wfdb_error("isigopen: record %s has no signals\n", record);
	if (navail <= 0)
	    return (navail);
    }

    /* If nsig <= 0, isigopen fills in up to (-nsig) members of siarray based
       on the contents of the header, but no signals are actually opened.  The
       value returned is the number of signals named in the header. */
    if (nsig <= 0) {
	nsig = -nsig;
	if (navail < nsig) nsig = navail;
	if (siarray != NULL)
	    for (s = 0; s < nsig; s++)
		siarray[s] = hsd[s]->info;
	in_msrec = 0;	/* necessary to avoid errors when reopening */
	return (navail);
    }

    /* Determine how many new signals we should attempt to open.  The caller's
       upper limit on this number is nsig, and the upper limit defined by the
       header is navail. */
    if (nsig > navail) nsig = navail;

    /* Allocate input signals and signal group workspace. */
    nn = nisig + nsig;
    if (allocisig(nn) != nn)
	return (-1);	/* failed, nisig is unchanged, allocisig emits error */
    else
	nsig = nn;
    nn = nigroups + hsd[nsig-nisig-1]->info.group + 1;
    if (allocigroup(nn) != nn)
	return (-1);	/* failed, allocigroup emits error */
    else
	ngroups = nn;

    /* Set default buffer size (if not set already by setibsize). */
    if (ibsize <= 0) ibsize = BUFSIZ;
  
    /* Open the signal files.  One signal group is handled per iteration.  In
       this loop, si counts through the entries that have been read from hsd,
       and s counts the entries that have been added to isd. */
    for (g = si = s = 0; si < navail && s < nsig; si = sj) {
        hs = hsd[si];
	is = isd[nisig+s];
	ig = igd[nigroups+g];

	/* Find out how many signals are in this group. */
        for (sj = si + 1; sj < navail; sj++)
	  if (hsd[sj]->info.group != hs->info.group) break;

	/* Skip this group if there are too few slots in the caller's array. */
	if (sj - si > nsig - s) continue;

	/* Set the buffer size and the seek capability flag. */
	if (hs->info.bsize < 0) {
	    ig->bsize = hs->info.bsize = -hs->info.bsize;
	    ig->seek = 0;
	}
	else {
	    if ((ig->bsize = hs->info.bsize) == 0) ig->bsize = ibsize;
	    ig->seek = 1;
	}

	/* Skip this group if a buffer can't be allocated. */
	if ((ig->buf = (char *)malloc(ig->bsize)) == NULL) continue;

	/* Check that the signal file is readable. */
	if (hs->info.fmt == 0)
	    ig->fp = NULL;	/* Don't open a file for a null signal. */
	else { 
	    ig->fp = wfdb_open(hs->info.fname, (char *)NULL, WFDB_READ);
	    /* Skip this group if the signal file can't be opened. */
	    if (ig->fp == NULL) {
	        (void)free(ig->buf);
		ig->buf = NULL;
		continue;
	    }
	}

	/* All tests passed -- fill in remaining data for this group. */
	ig->be = ig->bp = ig->buf + ig->bsize;
	ig->start = hs->start;
	ig->stat = 1;
	while (si < sj && s < nsig) {
	    if (copysi(&is->info, &hs->info) < 0) {
		wfdb_error("isigopen: insufficient memory\n");
		return (-3);
	    }
	    is->info.group = nigroups + g;
	    is->skew = hs->skew;
	    ++s;
	    if (++si < sj) {
		hs = hsd[si];
		is = isd[nisig + s];
	    }
	}
	g++;
    }

    /* Produce a warning message if none of the requested signals could be
       opened. */
    if (s == 0 && nsig)
	wfdb_error("isigopen: none of the signals for record %s is readable\n",
		 record);

    /* Copy the WFDB_Siginfo structures to the caller's array.  Use these
       data to construct the initial sample vector, and to determine the
       maximum number of samples per signal per frame and the maximum skew. */
    for (si = 0; si < s; si++) {
        is = isd[nisig + si];
	if (siarray != NULL && copysi(&siarray[si], &is->info) < 0) {
	    wfdb_error("isigopen: insufficient memory\n");
	    return (-3);
	}
	is->samp = is->info.initval;
	if (ispfmax < is->info.spf) ispfmax = is->info.spf;
	if (skewmax < is->skew) skewmax = is->skew;
    }
    setgvmode(gvmode);	/* Reset sfreq if appropriate. */
    gvc = ispfmax;	/* Initialize getvec's sample-within-frame counter. */
    nisig += s;		/* Update the count of open input signals. */
    nigroups += g;	/* Update the count of open input signal groups. */

    /* Determine the total number of samples per frame. */
    for (si = framelen = 0; si < nisig; si++)
	framelen += isd[si]->info.spf;

    /* Allocate workspace for getvec and isgsettime. */
    if ((tvector = calloc(sizeof(WFDB_Sample), framelen)) == NULL ||
	(uvector = calloc(sizeof(WFDB_Sample), framelen)) == NULL) {
	wfdb_error("isigopen: can't allocate frame buffer\n");
	if (tvector) (void)free(tvector);
	return (-3);
    }

    /* If deskewing is required, allocate the deskewing buffer (unless this is
       a multi-segment record and dsbuf has been allocated already). */
    if (skewmax != 0 && (!in_msrec || dsbuf == NULL)) {
	dsbi = -1;	/* mark buffer contents as invalid */
	dsblen = framelen * (skewmax + 1);
	if (dsbuf) free(dsbuf);
	if ((dsbuf=(WFDB_Sample *)malloc(dsblen*sizeof(WFDB_Sample))) == NULL)
	    wfdb_error("isigopen: can't allocate buffer for deskewing\n");
	/* If the buffer couldn't be allocated, the signals can still be read,
	   but won't be deskewed. */
    }
    return (s);
}

FINT osigopen(record, siarray, nsig)
char *record;
WFDB_Siginfo *siarray;
unsigned int nsig;
{
    int n;
    struct osdata *os, *op;
    struct ogdata *og;
    WFDB_Signal s;
    unsigned int buflen, ga;

    /* Close previously opened output signals unless otherwise requested. */
    if (*record == '+') record++;
    else osigclose();

    if ((n = readheader(record)) < 0)
	return (n);
    if (n < nsig) {
	wfdb_error("osigopen: record %s has fewer signals than needed\n",
		 record);
	return (-3);
    }

    /* Allocate workspace for output signals. */
    if (allocosig(nosig + nsig) < 0) return (-3);
    /* Allocate workspace for output signal groups. */
    if (allocogroup(nogroups + hsd[nsig-1]->info.group + 1) < 0) return (-3);

    /* Initialize local variables. */
    if (obsize <= 0) obsize = BUFSIZ;

    /* Set the group number adjustment.  This quantity is added to the group
       numbers of signals which are opened below;  it accounts for any output
       signals which were left open from previous calls. */
    ga = nogroups;

    /* Open the signal files.  One signal is handled per iteration. */
    for (s = 0, os = osd[nosig]; s < nsig; s++, nosig++, siarray++) {
	op = os;
	os = osd[nosig];

	/* Copy signal information from readheader's workspace. */
	if (copysi(&os->info, &hsd[s]->info) < 0 ||
	    copysi(siarray, &hsd[s]->info) < 0) {
	    wfdb_error("osigopen: insufficient memory\n");
	    return (-3);
	}
	if (os->info.spf < 1) os->info.spf = siarray->spf = 1;
	os->info.cksum = siarray->cksum = 0;
	os->info.nsamp = siarray->nsamp = (WFDB_Time)0L;
	os->info.group += ga; siarray->group += ga;

	if (s == 0 || os->info.group != op->info.group) {
	    /* This is the first signal in a new group; allocate buffer. */
	    size_t obuflen;

	    og = ogd[os->info.group];
	    og->bsize = os->info.bsize;
	    obuflen = og->bsize ? og->bsize : obsize;
	    if ((og->buf = (char *)malloc(obuflen)) == NULL) {
	        wfdb_error("osigopen: can't allocate buffer for %s\n",
			   os->info.fname);
		return (-3);
	    }
	    og->bp = og->buf;
	    og->be = og->buf + obuflen;
	    if (os->info.fmt == 0)
	        og->fp = NULL;	/* don't open a file for a null signal */
	    /* An error in opening an output file is fatal. */
	    else {
		og->fp = wfdb_open(os->info.fname,(char *)NULL, WFDB_WRITE);
		if (og->fp == NULL) {
		    wfdb_error("osigopen: can't open %s\n", os->info.fname);
		    free(og->buf);
		    og->buf = NULL;
		    osigclose();
		    return (-3);
		}
	    }
	    nogroups++;
	}
	else {
	    /* This signal belongs to the same group as the previous signal. */
	    if (os->info.fmt != op->info.fmt ||
		os->info.bsize != op->info.bsize) {
		wfdb_error(
		    "osigopen: error in specification of signal %d or %d\n",
		     s-1, s);
		return (-2);
	    }
	}
    }
    return (s);
}

FINT osigfopen(siarray, nsig)
WFDB_Siginfo *siarray;
unsigned int nsig;
{
    struct osdata *os, *op;
    struct ogdata *og;
    int s;
    WFDB_Siginfo *si;

    /* Close any open output signals. */
    osigclose();

    /* Do nothing further if there are no signals to open. */
    if (siarray == NULL || nsig == 0) return (0);

    if (obsize <= 0) obsize = BUFSIZ;

    /* Prescan siarray to check the signal specifications and to determine
       the number of signal groups. */
    for (s = 0, si = siarray; s < nsig; s++, si++) {
	/* The combined lengths of the fname and desc strings should be 80
	   characters or less, the bsize field must not be negative, the
	   format should be legal, group numbers should be the same if and
	   only if file names are the same, and group numbers should begin
	   at zero and increase in steps of 1. */
	if (strlen(si->fname) + strlen(si->desc) > 80 ||
	    si->bsize < 0 || !isfmt(si->fmt)) {
	    wfdb_error("osigfopen: error in specification of signal %d\n",
		       s);
	    return (-2);
	}
	if (!((s == 0 && si->group == 0) ||
	    (s && si->group == (si-1)->group &&
	     strcmp(si->fname, (si-1)->fname) == 0) ||
	    (s && si->group == (si-1)->group + 1 &&
	     strcmp(si->fname, (si-1)->fname) != 0))) {
	    wfdb_error(
		     "osigfopen: incorrect file name or group for signal %d\n",
		     s);
	    return (-2);
	}
    }

    /* Allocate workspace for output signals. */
    if (allocosig(nsig) < 0) return (-3);
    /* Allocate workspace for output signal groups. */
    if (allocogroup((si-1)->group + 1) < 0) return (-3);

    /* Open the signal files.  One signal is handled per iteration. */
    for (os = osd[0]; nosig < nsig; nosig++, siarray++) {

	op = os;
	os = osd[nosig];
	/* Check signal specifications.  The combined lengths of the fname
	   and desc strings should be 80 characters or less, the bsize field
	   must not be negative, the format should be legal, group numbers
	   should be the same if and only if file names are the same, and
	   group numbers should begin at zero and increase in steps of 1. */
	if (strlen(siarray->fname) + strlen(siarray->desc) > 80 ||
	    siarray->bsize < 0 || !isfmt(siarray->fmt)) {
	    wfdb_error("osigfopen: error in specification of signal %d\n",
		       nosig);
	    return (-2);
	}
	if (!((nosig == 0 && siarray->group == 0) ||
	    (nosig && siarray->group == (siarray-1)->group &&
	     strcmp(siarray->fname, (siarray-1)->fname) == 0) ||
	    (nosig && siarray->group == (siarray-1)->group + 1 &&
	     strcmp(siarray->fname, (siarray-1)->fname) != 0))) {
	    wfdb_error(
		     "osigfopen: incorrect file name or group for signal %d\n",
		     nosig);
	    return (-2);
	}

	/* Copy signal information from the caller's array. */
	if (copysi(&os->info, siarray) < 0) {
	    wfdb_error("osigfopen: insufficient memory\n");
	    return (-3);
	}
	if (os->info.spf < 1) os->info.spf = 1;
	os->info.cksum = 0;
	os->info.nsamp = (WFDB_Time)0L;

	/* Check if this signal is in the same group as the previous one. */
	if (nosig == 0 || os->info.group != op->info.group) {
	    size_t obuflen;

	    og = ogd[os->info.group];
	    og->bsize = os->info.bsize;
	    obuflen = og->bsize ? og->bsize : obsize;
	    /* This is the first signal in a new group; allocate buffer. */
	    if ((og->buf = (char *)malloc(obuflen)) == NULL) {
	        wfdb_error("osigfopen: can't allocate buffer for %s\n",
			   os->info.fname);
		return (-3);
	    }
	    og->bp = og->buf;
	    og->be = og->buf + obuflen;
	    if (os->info.fmt == 0)
	        og->fp = NULL;   /* don't open a file for a null signal */
	    /* An error in opening an output file is fatal. */
	    else {
	        og->fp = wfdb_open(os->info.fname,(char *)NULL, WFDB_WRITE);
		if (og->fp == NULL) {
		    wfdb_error("osigfopen: can't open %s\n", os->info.fname);
		    free(og->buf);
		    og->buf = NULL;
		    osigclose();
		    return (-3);
		}
	    }
	    nogroups++;
	}
	else {
	    /* This signal belongs to the same group as the previous signal. */
	    if (os->info.fmt != op->info.fmt ||
		os->info.bsize != op->info.bsize) {
		wfdb_error(
		    "osigfopen: error in specification of signal %d or %d\n",
		     nosig-1, nosig);
		return (-2);
	    }
	}
    }

    return (nosig);
}

/* Function getvec can operate in two different modes when reading
multifrequency records.  In WFDB_LOWRES mode, getvec returns one sample of each
signal per frame (decimating any oversampled signal by returning the average of
all of its samples within the frame).  In WFDB_HIGHRES mode, each sample of any
oversampled signal is returned by successive invocations of getvec; samples of
signals sampled at lower frequencies are returned on two or more successive
invocations of getvec as appropriate.  Function setgvmode can be used to change
getvec's operating mode, which is determined by environment variable
WFDBGVMODE or constant DEFWFDBGVMODE by default.  When reading
ordinary records (with all signals sampled at the same frequency), getvec's
behavior is independent of its operating mode.

Since sfreq and ffreq are always positive, the effect of adding 0.5 to their
quotient before truncating it to an int (see below) is to round the quotient
to the nearest integer.  Although sfreq should always be an exact multiple
of ffreq, loss of precision in representing non-integer frequencies can cause
problems if this rounding is omitted.  Thanks to Guido Muesch for pointing out
this problem.
 */

FINT getspf()
{
    return ((sfreq != ffreq) ? (int)(sfreq/ffreq + 0.5) : 1);
}

FVOID setgvmode(mode)
int mode;
{
    if (mode < 0) {	/* (re)set to default mode */
	char *p;

        if (p = getenv("WFDBGVMODE"))
	    mode = atoi(p);
	else
	    mode = DEFWFDBGVMODE;
    }

    if (mode == WFDB_HIGHRES) {
	gvmode = WFDB_HIGHRES;
	sfreq = ffreq * ispfmax;
    }
    else {
	gvmode = WFDB_LOWRES;
	sfreq = ffreq;
    }
}

/* An application can specify the input sampling frequency it prefers by
   calling setifreq after opening the input record. */

static long mticks, nticks, mnticks;
static int rgvstat;
static WFDB_Time rgvtime, gvtime;
static WFDB_Sample *gv0, *gv1;

FINT setifreq(f)
WFDB_Frequency f;
{
    if (f > 0.0) {
	WFDB_Frequency error, g = sfreq;

	gv0 = realloc(gv0, nisig*sizeof(WFDB_Sample));
	gv1 = realloc(gv1, nisig*sizeof(WFDB_Sample));
	if (gv0 == NULL || gv1 == NULL) {
	    wfdb_error("setifreq: too many (%d) input signals\n", nisig);
	    if (gv0) (void)free(gv0);
	    ifreq = 0.0;
	    return (-2);
	}
	ifreq = f;
	/* The 0.005 below is the maximum tolerable error in the resampling
	   frequency (in Hz).  The code in the while loop implements Euclid's
	   algorithm for finding the greatest common divisor of two integers,
	   but in this case the integers are (implicit) multiples of 0.005. */
	while ((error = f - g) > 0.005 || error < -0.005)
	    if (f > g) f -= g;
	    else g -= f;
	/* f is now the GCD of sfreq and ifreq in the sense described above.
	   We divide each raw sampling interval into mticks subintervals. */
        mticks = (long)(sfreq/f + 0.5);
	/* We divide each resampled interval into nticks subintervals. */
	nticks = (long)(ifreq/f + 0.5);
	/* Raw and resampled intervals begin simultaneously once every mnticks
	   subintervals; we say an epoch begins at these times. */
	mnticks = mticks * nticks;
	/* gvtime is the number of subintervals from the beginning of the
	   current epoch to the next sample to be returned by getvec(). */
	gvtime = 0;
	rgvstat = rgetvec(gv0);
	rgvstat = rgetvec(gv1);
	/* rgvtime is the number of subintervals from the beginning of the
	   current epoch to the most recent sample returned by rgetvec(). */
	rgvtime = nticks;
	return (0);
    }
    else {
	ifreq = 0.0;
	wfdb_error("setifreq: improper frequency %g (must be > 0)\n", f);
	return (-1);
    }
}

FFREQUENCY getifreq(void)
{
    return (ifreq > (WFDB_Frequency)0 ? ifreq : sfreq);
}    

FINT getvec(vector)
WFDB_Sample *vector;
{
    int i;

    if (ifreq == 0.0 || ifreq == sfreq)	/* no resampling necessary */
	return (rgetvec(vector));

    /* Resample the input. */
    if (rgvtime > mnticks) {
	rgvtime -= mnticks;
	gvtime  -= mnticks;
    }
    while (gvtime > rgvtime) {
	for (i = 0; i < nisig; i++)
	    gv0[i] = gv1[i];
	rgvstat = rgetvec(gv1);
	rgvtime += nticks;
    }
    for (i = 0; i < nisig; i++) {
	vector[i] = gv0[i] + (gvtime%nticks)*(gv1[i]-gv0[i])/nticks;
        gv0[i] = gv1[i];
    }
    gvtime += mticks;
    return (rgvstat);
}

FINT getframe(vector)
WFDB_Sample *vector;
{
    int stat;

    if (dsbuf) {	/* signals must be deskewed */
	int c, i, j, s;

	/* First, obtain the samples needed. */
	if (dsbi < 0) {	/* dsbuf contents are invalid -- refill dsbuf */
	    for (dsbi = i = 0; i < dsblen; dsbi = i += framelen)
		stat = getskewedframe(dsbuf + dsbi);
	    dsbi = 0;
	}
	else {		/* replace oldest frame in dsbuf only */
	    stat = getskewedframe(dsbuf + dsbi);
	    if ((dsbi += framelen) >= dsblen) dsbi = 0;
	}
	/* Assemble the deskewed frame from the data in dsbuf. */
	for (j = s = 0; s < nisig; s++) {
	    if ((i = j + dsbi + isd[s]->skew*framelen) >= dsblen) i -= dsblen;
	    for (c = 0; c < isd[s]->info.spf; c++)
		vector[j++] = dsbuf[i++];
	}
    }
    else		/* no deskewing necessary */
	stat = getskewedframe(vector);
    istime++;
    return (stat);
}

FINT putvec(vector)
WFDB_Sample *vector;
{
    int c, dif, stat = (int)nosig;
    struct osdata *os;
    struct ogdata *og;
    WFDB_Signal s;
    WFDB_Group g;

    for (s = 0; s < nosig; s++) {
	os = osd[s];
	g = os->info.group;
	og = ogd[os->info.group];
	if (os->info.nsamp++ == (WFDB_Time)0L)
	    os->info.initval = os->samp = *vector;
	for (c = 0; c < os->info.spf; c++, vector++) {
	    switch (os->info.fmt) {
	      case 0:	/* null signal (do not write) */
		os->samp = *vector; break;
	      case 8:	/* 8-bit first differences */
	      default:
		/* Handle large slew rates sensibly. */
		if ((dif = *vector - os->samp) < -128) { dif = -128; stat=0; }
		else if (dif > 127) { dif = 127; stat = 0; }
		os->samp += dif;
		w8(dif, og);
		break;
	      case 16:	/* 16-bit amplitudes */
		w16(*vector, og); os->samp = *vector; break;
	      case 61:	/* 16-bit amplitudes, bytes swapped */
		w61(*vector, og); os->samp = *vector; break;
	      case 80:	/* 8-bit offset binary amplitudes */
		w80(*vector, og); os->samp = *vector; break;
	      case 160:	/* 16-bit offset binary amplitudes */
		w160(*vector, og); os->samp = *vector; break;
	      case 212:	/* 2 12-bit amplitudes bit-packed in 3 bytes */
		w212(*vector, og); os->samp = *vector; break;
	      case 310:	/* 3 10-bit amplitudes bit-packed in 4 bytes */
		w310(*vector, og); os->samp = *vector; break;
	      case 311:	/* 3 10-bit amplitudes bit-packed in 4 bytes */
		w311(*vector, og); os->samp = *vector; break;
	    }
	    if (wfdb_ferror(og->fp)) {
		wfdb_error("putvec: write error in signal %d\n", s);
		stat = -1;
	    }
	    else
		os->info.cksum += os->samp;
	}
    }
    ostime++;
    return (stat);
}

FINT isigsettime(t)
WFDB_Time t;
{
    WFDB_Group g;
    int stat = 0;
    WFDB_Signal s;
	
    /* Return immediately if no seek is needed. */
    if (t == istime || nisig == 0) return (0);

    for (g = 1; g < nigroups; g++)
        if ((stat = isgsettime(g, t)) < 0) break;
    /* Seek on signal group 0 last (since doing so updates istime and would
       confuse isgsettime if done first). */
    if (stat == 0) stat = isgsettime(0, t);
    return (stat);
}
    
FINT isgsettime(g, t)
WFDB_Group g;
WFDB_Time t;
{
    int spf, stat, tr, trem = 0;

    /* Handle negative arguments as equivalent positive arguments. */
    if (t < 0L) t = -t;

    tr = t;

    /* Convert t to raw sample intervals if we are resampling. */
    if (ifreq > (WFDB_Frequency)0)
	t = (WFDB_Time)(t * sfreq/ifreq);

    /* If we're in WFDB_HIGHRES mode, convert t from samples to frames, and
       save the remainder (if any) in trem. */
    if (sfreq != ffreq) {
	spf = (int)(sfreq/ffreq);
	trem = t % spf;
	t /= spf;
    }

    if ((stat = isgsetframe(g, t)) == 0 && g == 0) {
	while (trem-- > 0) {
	    if (rgetvec(uvector) < 0) {
		wfdb_error("isigsettime: improper seek on signal group %d\n",
			   g);
		return (-1);
	    }
	}
	if (ifreq > (WFDB_Frequency)0 && ifreq != sfreq) {
	    gvtime = 0;
	    rgvstat = rgetvec(gv0);
	    rgvstat = rgetvec(gv1);
	    rgvtime = nticks;
	}
    }

    if (ifreq > (WFDB_Frequency)0 && tr != t) {
	t = (WFDB_Time)(t * ifreq/sfreq);

	while (t++ < tr)
	    getvec(uvector);
    }

    return (stat);
}

FINT setibsize(n)
int n;
{
    if (nisig) {
	wfdb_error("setibsize: can't change buffer size after isigopen\n");
	return (-1);
    }
    if (n < 0) {
	wfdb_error("setibsize: illegal buffer size %d\n", n);
	return (-2);
    }
    if (n == 0) n = BUFSIZ;
    return (ibsize = n);
}

FINT setobsize(n)
int n;
{
    if (nosig) {
	wfdb_error("setobsize: can't change buffer size after osig[f]open\n");
	return (-1);
    }
    if (n < 0) {
	wfdb_error("setobsize: illegal buffer size %d\n", n);
	return (-2);
    }
    if (n == 0) n = BUFSIZ;
    return (obsize = n);
}

FINT newheader(record)
char *record;
{
    int stat;
    WFDB_Signal s;
    WFDB_Siginfo *osi;

    if ((osi = malloc(nosig*sizeof(WFDB_Siginfo))) == NULL) {
	wfdb_error("newheader: insufficient memory\n");
	return (-1);
    }
    for (s = 0; s < nosig; s++)
	if (copysi(&osi[s], &osd[s]->info) < 0) {
 	    wfdb_error("newheader: insufficient memory\n");
	    return (-1);
	}
    stat = setheader(record, osi, nosig);
    for (s = 0; s < nosig; s++) {
      if (osi[s].fname) (void)free(osi[s].fname);
      if (osi[s].desc) (void)free(osi[s].desc);
      if (osi[s].units) (void)free(osi[s].units);
    }
    (void)free(osi);
    return (stat);
}

FINT setheader(record, siarray, nsig)
char *record;
WFDB_Siginfo *siarray;
unsigned int nsig;
{
    char *p;
    WFDB_Signal s;

    /* If another output header file was opened, close it. */
    if (oheader) {
	(void)wfdb_fclose(oheader);
	oheader = NULL;
    }

    /* Quit (with message from wfdb_checkname) if name is illegal. */
    if (wfdb_checkname(record, "record"))
	return (-1);

    /* Try to create the header file. */
    if ((oheader = wfdb_open("hea", record, WFDB_WRITE)) == NULL) {
	wfdb_error("newheader: can't create header for record %s\n", record);
	return (-1);
    }

    /* Write the general information line. */
    (void)wfdb_fprintf(oheader, "%s %d %g", record, nsig, ffreq);
    if ((cfreq > 0.0 && cfreq != ffreq) || bcount != 0.0) {
	(void)wfdb_fprintf(oheader, "/%g", cfreq);
	if (bcount != 0.0)
	    (void)wfdb_fprintf(oheader, "(%g)", bcount);
    }
    (void)wfdb_fprintf(oheader, " %ld", nsig > 0 ? siarray[0].nsamp : 0L);
    if (btime != 0L || bdate != (WFDB_Date)0) {
	if (btime == 0L)
	    (void)wfdb_fprintf(oheader, " 0:00");
        else if (btime % 1000 == 0)
	    (void)wfdb_fprintf(oheader, " %s",
			   timstr((WFDB_Time)(btime*sfreq/1000.0)));
	else
	    (void)wfdb_fprintf(oheader, " %s",
			   mstimstr((WFDB_Time)(btime*sfreq/1000.0)));
    }
    if (bdate)
	(void)wfdb_fprintf(oheader, "%s", datstr(bdate));
    (void)wfdb_fprintf(oheader, "\r\n");

    /* Write a signal specification line for each signal. */
    for (s = 0; s < nsig; s++) {
	(void)wfdb_fprintf(oheader, "%s %d", siarray[s].fname, siarray[s].fmt);
	if (siarray[s].spf > 1)
	    (void)wfdb_fprintf(oheader, "x%d", siarray[s].spf);
	if (osd && osd[s]->skew)
	    (void)wfdb_fprintf(oheader, ":%d", osd[s]->skew*siarray[s].spf);
	if (ogd && ogd[osd[s]->info.group]->start)
	    (void)wfdb_fprintf(oheader, "+%ld",
			       ogd[osd[s]->info.group]->start);
	else if (prolog_bytes)
	    (void)wfdb_fprintf(oheader, "+%ld", prolog_bytes);
	(void)wfdb_fprintf(oheader, " %g", siarray[s].gain);
	if (siarray[s].baseline != siarray[s].adczero)
	    (void)wfdb_fprintf(oheader, "(%d)", siarray[s].baseline);
	if (siarray[s].units && (p = strtok(siarray[s].units, " \t\n\r")))
	    (void)wfdb_fprintf(oheader, "/%s", p);
	(void)wfdb_fprintf(oheader, " %d %d %d %d %d",
		     siarray[s].adcres, siarray[s].adczero, siarray[s].initval,
		     (short int)(siarray[s].cksum & 0xffff), siarray[s].bsize);
	if (siarray[s].desc && (p = strtok(siarray[s].desc, "\n\r")))
	    (void)wfdb_fprintf(oheader, " %s", p);
	(void)wfdb_fprintf(oheader, "\r\n");
    }
    prolog_bytes = 0L;
    (void)wfdb_fflush(oheader);
    return (0);
}

FINT setmsheader(record, segment_name, nsegments)
char *record, *segment_name[];
unsigned nsegments;
{
    WFDB_Frequency msfreq, mscfreq;
    double msbcount;
    int n, nsig, old_in_msrec = in_msrec;
    long *ns;
    unsigned i;

    isigclose();	/* close any open input signals */

    /* If another output header file was opened, close it. */
    if (oheader) {
	(void)wfdb_fclose(oheader);
	oheader = NULL;
    }

    /* Quit (with message from wfdb_checkname) if name is illegal. */
    if (wfdb_checkname(record, "record"))
	return (-1);

    if (nsegments < 1) {
	wfdb_error("setmsheader: record must contain at least one segment\n");
	return (-1);
    }

    if ((ns = (long *)malloc((unsigned)(sizeof(long)*nsegments))) == NULL) {
	wfdb_error("setmsheader: insufficient memory\n");
	return (-2);
    }

    for (i = 0; i < nsegments; i++) {
	if (strlen(segment_name[i]) > WFDB_MAXRNL) {
	    wfdb_error(
	     "setmsheader: `%s' is too long for a segment name in record %s\n",
		     segment_name[i], record);
	    (void)free(ns);
	    return (-2);
	}
	in_msrec = 1;
	n = readheader(segment_name[i]);
	in_msrec = old_in_msrec;
	if (n < 0) {
	    wfdb_error("setmsheader: can't read segment %s header\n",
		     segment_name[i]);
	    (void)free(ns);
	    return (-3);
	}
	if ((ns[i] = hsd[0]->info.nsamp) <= 0L) {
	    wfdb_error("setmsheader: length of segment %s must be specified\n",
		     segment_name[i]);
	    (void)free(ns);
	    return (-4);
	}
	if (i == 0) {
	    nsig = n;
	    msfreq = ffreq;
	    mscfreq = cfreq;
	    msbcount = bcount;
	    msbtime = btime;
	    msbdate = bdate;
	    msnsamples = ns[i];
	}
	else {
	    if (nsig != n) {
		wfdb_error(
		    "setmsheader: incorrect number of signals in segment %s\n",
			 segment_name[i]);
		(void)free(ns);
		return (-4);
	    }
	    if (msfreq != ffreq) {
		wfdb_error(
		   "setmsheader: incorrect sampling frequency in segment %s\n",
			 segment_name[i]);
		(void)free(ns);
		return (-4);
	    }
	    msnsamples += ns[i];
	}
    }

    /* Try to create the header file. */
    if ((oheader = wfdb_open("hea", record, WFDB_WRITE)) == NULL) {
	wfdb_error("setmsheader: can't create header file for record %s\n",
		 record);
	(void)free(ns);
	return (-1);
    }

    /* Write the first line of the master header. */
    (void)wfdb_fprintf(oheader,"%s/%u %d %g", record, nsegments, nsig, msfreq);
    if ((mscfreq > 0.0 && mscfreq != msfreq) || msbcount != 0.0) {
	(void)wfdb_fprintf(oheader, "/%g", mscfreq);
	if (msbcount != 0.0)
	    (void)wfdb_fprintf(oheader, "(%g)", msbcount);
    }
    (void)wfdb_fprintf(oheader, " %ld", msnsamples);
    if (msbtime != 0L || msbdate != (WFDB_Date)0) {
        if (msbtime % 1000 == 0)
	    (void)wfdb_fprintf(oheader, " %s",
			   timstr((WFDB_Time)(msbtime*sfreq/1000.0)));
	else
	    (void)wfdb_fprintf(oheader, " %s",
			   mstimstr((WFDB_Time)(msbtime*sfreq/1000.0)));
    }
    if (msbdate)
	(void)wfdb_fprintf(oheader, "%s", datstr(msbdate));
    (void)wfdb_fprintf(oheader, "\r\n");

    /* Write a line for each segment. */
    for (i = 0; i < nsegments; i++)
	(void)wfdb_fprintf(oheader, "%s %ld\r\n", segment_name[i], ns[i]);

    (void)free(ns);
    return (0);
}

FINT wfdbgetskew(s)
WFDB_Signal s;
{
    if (s < nisig)
	return (isd[s]->skew);
    else
	return (0);
}

/* Careful!!  This function is dangerous, and should be used only to restore
   skews when they have been reset as a side effect of using, e.g., sampfreq */
FVOID wfdbsetiskew(s, skew)
WFDB_Signal s;
int skew;
{
    if (s < nisig)
        isd[s]->skew = skew;
}

/* Note: wfdbsetskew affects *only* the skew to be written by setheader.
   It does not affect how getframe deskews input signals, nor does it
   affect the value returned by wfdbgetskew. */
FVOID wfdbsetskew(s, skew)
WFDB_Signal s;
int skew;
{
    if (s < nosig)
	osd[s]->skew = skew;
}

FLONGINT wfdbgetstart(s)
WFDB_Signal s;
{
    if (s < nisig)
        return (igd[isd[s]->info.group]->start);
    else
	return (0L);
}

/* Note: wfdbsetstart affects *only* the byte offset to be written by
   setheader.  It does not affect how isgsettime calculates byte offsets, nor
   does it affect the value returned by wfdbgetstart. */
FVOID wfdbsetstart(s, bytes)
WFDB_Signal s;
long bytes;
{
    if (s < nosig)
        ogd[osd[s]->info.group]->start = bytes;
    prolog_bytes = bytes;
}

FSTRING getinfo(record)
char *record;
{
    char *p;
    static char linebuf[256];

    if (record != NULL && readheader(record) < 0) {
	wfdb_error("getinfo: can't read record %s header\n", record);
	return (NULL);
    }
    else if (record == NULL && hheader == NULL) {
	wfdb_error("getinfo: caller did not specify record name\n");
	return (NULL);
    }

    /* Find a line beginning with '#'. */
    do {
	if (wfdb_fgets(linebuf, 256, hheader) == NULL)
	    return (NULL);
    } while (linebuf[0] != '#');

    /* Strip off trailing newline (and '\r' if present). */
    p = linebuf + strlen(linebuf) - 1;
    if (*p == '\n') *p-- = '\0';
    if (*p == '\r') *p = '\0';
    return (linebuf+1);
}

FINT putinfo(s)
char *s;
{
    if (oheader == NULL) {
	wfdb_error(
                "putinfo: header not initialized (call `newheader' first)\n");
	return (-1);
    }
    (void)wfdb_fprintf(oheader, "#%s\r\n", s);
    (void)wfdb_fflush(oheader);
    return (0);
}

FFREQUENCY sampfreq(record)
char *record;
{
    int n;

    if (record != NULL) {
	/* Save the current record name. */
	wfdb_setirec(record);
	/* Don't require the sampling frequency of this record to match that
	   of the previously opened record, if any.  (readheader will
	   complain if the previously defined sampling frequency was > 0.) */
	setsampfreq(0.);
	/* readheader sets sfreq if successful. */
	if ((n = readheader(record)) < 0)
	    /* error message will come from readheader */
	    return ((WFDB_Frequency)n);
    }
    return (sfreq);
}

FINT setsampfreq(freq)
WFDB_Frequency freq;
{
    if (freq >= 0.) {
	sfreq = ffreq = freq;
	if (gvmode == WFDB_HIGHRES) sfreq *= ispfmax;
	return (0);
    }
    wfdb_error("setsampfreq: sampling frequency must not be negative\n");
    return (-1);
}

static char date_string[12] = "";
static char time_string[30];

#ifndef __STDC__
#ifndef _WINDOWS
typedef long time_t;
#endif
#endif

FINT setbasetime(string)
char *string;
{
    char *p;

    if (string == NULL || *string == '\0') {
#ifndef NOTIME
	struct tm *now;
	time_t t;

	t = time((time_t *)NULL);    /* get current time from system clock */
	now = localtime(&t);
	(void)sprintf(date_string, "%02d/%02d/%d",
		now->tm_mday, now->tm_mon+1, now->tm_year+1900);
	bdate = strdat(date_string);
	(void)sprintf(time_string, "%d:%d:%d",
		now->tm_hour, now->tm_min, now->tm_sec);
	btime = (long)(strtim(time_string)*1000.0/sfreq);
#endif
	return (0);
    }
    while (*string == ' ') string++;
    if (p = strchr(string, ' '))
        *p++ = '\0';	/* split time and date components */
    btime = strtim(string);
    bdate = p ? strdat(p) : (WFDB_Date)0;
    if (btime == 0L && bdate == (WFDB_Date)0 && *string != '[') {
	if (p) *(--p) = ' ';
	wfdb_error("setbasetime: incorrect time format, '%s'\n", string);
	return (-1);
    }
    btime *= 1000.0/sfreq;
    return (0);
}

FSTRING timstr(t)
WFDB_Time t;
{
    char *p;

    p = strtok(mstimstr(t), ".");		 /* discard msec field */
    if (t <= 0L && (btime != 0L || bdate != (WFDB_Date)0)) { /* time of day */
	(void)strcat(p, date_string);		  /* append dd/mm/yyyy */
	(void)strcat(p, "]");
    }
    return (p);	
}

static WFDB_Date pdays = -1;

FSTRING mstimstr(t)
WFDB_Time t;
{
    double f;
    int hours, minutes, seconds, msec;
    WFDB_Date days;
    long s;

    if (ifreq > 0.) f = ifreq;
    else if (sfreq > 0.) f = sfreq;
    else f = 1.0;

    if (t > 0L || (btime == 0L && bdate == (WFDB_Date)0)) { /* time interval */
	if (t < 0L) t = -t;
	/* Convert from sample intervals to seconds. */
	s = t / f;
	msec = (t - s*f)*1000/f;
	t = s;
	seconds = t % 60;
	t /= 60;
	minutes = t % 60;
	hours = t / 60;
	if (hours > 0)
	    (void)sprintf(time_string, "%2d:%02d:%02d.%03d",
			  hours, minutes, seconds, msec);
	else
	    (void)sprintf(time_string, "   %2d:%02d.%03d",
			  minutes, seconds, msec);
    }
    else {			/* time of day */
	/* Convert to sample intervals since midnight. */
	t = (WFDB_Time)(btime*sfreq/1000.0) - t;
	/* Convert from sample intervals to seconds. */
	s = t / f;
	msec = (t - s*f)*1000/f;
	t = s;
	seconds = t % 60;
	t /= 60;
	minutes = t % 60;
	t /= 60;
	hours = t % 24;
	days = t / 24;
	if (days != pdays) {
	    if (bdate > 0)
		(void)datstr((pdays = days) + bdate);
	    else if (days == 0)
		date_string[0] = '\0';
	    else
		(void)sprintf(date_string, " %ld", days);
	}
	(void)sprintf(time_string, "[%02d:%02d:%02d.%03d%s]",
		      hours, minutes, seconds, msec, date_string);
    }
    return (time_string);
}

FFREQUENCY getcfreq()
{
    return (cfreq > 0. ? cfreq : ffreq);
}

FVOID setcfreq(freq)
WFDB_Frequency freq;
{
    cfreq = freq;
}

FDOUBLE getbasecount()
{
    return (bcount);
}

FVOID setbasecount(counter)
double counter;
{
    bcount = counter;
}

FSITIME strtim(string)
char *string;
{
    char *p;
    double f, x, y, z;
    WFDB_Date days = 0L;
    WFDB_Time t;

    if (ifreq > 0.) f = ifreq;
    else if (sfreq > 0.) f = sfreq;
    else f = 1.0;
    while (*string==' ' || *string=='\t' || *string=='\n' || *string=='\r')
	string++;
    switch (*string) {
      case 'c': return (cfreq > 0. ?
			(WFDB_Time)((atof(string+1)-bcount)*f/cfreq) :
			(WFDB_Time)atol(string+1));
      case 'e':	return ((in_msrec ? msnsamples : nsamples) * 
		        ((gvmode == WFDB_HIGHRES) ? ispfmax : 1));
      case 'f': return ((WFDB_Time)(atol(string+1)*f/ffreq));
      case 'i':	return (istime * ((gvmode == WFDB_HIGHRES) ? ispfmax : 1));
      case 'o':	return (ostime);
      case 's':	return ((WFDB_Time)atol(string+1));
      case '[':	  /* time of day, possibly with date or days since start */
	if (p = strchr(string, ' ')) {
	    if (strchr(p, '/')) days = strdat(p) - bdate;
	    else days = atol(p+1);
	}
	t = strtim(string+1) - (WFDB_Time)(btime*f/1000.0);
	if (days > 0L) t += (WFDB_Time)(days*24*60*60*f);
	return (-t);
      default:
	x = atof(string);
	if ((p = strchr(string, ':')) == NULL) return ((long)(x*f + 0.5));
	y = atof(++p);
	if ((p = strchr(p, ':')) == NULL) return ((long)((60.*x + y)*f + 0.5));
	z = atof(++p);
	return ((WFDB_Time)((3600.*x + 60.*y + z)*f + 0.5));
    }
}

/* The functions datstr and strdat convert between Julian dates (used
   internally) and dd/mm/yyyy format dates.  (Yes, this is overkill for this
   application.  For the astronomically-minded, Julian dates are supposed
   to begin at noon GMT, but these begin at midnight local time.)  They are
   based on similar functions in chapter 1 of "Numerical Recipes", by Press,
   Flannery, Teukolsky, and Vetterling (Cambridge U. Press, 1986). */

FSTRING datstr(date)
WFDB_Date date;
{
    int d, m, y, gcorr, jm, jy;
    long jd;

    if (date >= 2299161L) {	/* Gregorian calendar correction */
	gcorr = ((date - 1867216L) - 0.25)/36524.25;
	date += 1 + gcorr - (long)(0.25*gcorr);
    }
    date += 1524;
    jy = 6680 + ((date - 2439870L) - 122.1)/365.25;
    jd = 365L*jy + (0.25*jy);
    jm = (date - jd)/30.6001;
    d = date - jd - (int)(30.6001*jm);
    if ((m = jm - 1) > 12) m -= 12;
    y = jy - 4715;
    if (m > 2) y--;
    if (y <= 0) y--;
    (void)sprintf(date_string, " %02d/%02d/%d", d, m, y);
    return (date_string);
}

FDATE strdat(string)
char *string;
{
    char *mp, *yp;
    int d, m, y, gcorr, jm, jy;
    WFDB_Date date;

    if ((mp = strchr(string,'/')) == NULL || (yp = strchr(mp+1,'/')) == NULL ||
	(d = atoi(string)) < 1 || d > 31 || (m = atoi(mp+1)) < 1 || m > 12 ||
	(y = atoi(yp+1)) == 0)
	return (0L);
    if (m > 2) { jy = y; jm = m + 1; }
    else { jy = y - 1; 	jm = m + 13; }
    if (jy > 0) date = 365.25*jy;
    else date = -(long)(-365.25 * (jy + 0.25));
    date += (int)(30.6001*jm) + d + 1720995L;
    if (d + 31L*(m + 12L*y) >= (15 + 31L*(10 + 12L*1582))) { /* 15/10/1582 */
	gcorr = 0.01*jy;		/* Gregorian calendar correction */
	date += 2 - gcorr + (int)(0.25*gcorr);
    }
    return (date);
}

FINT adumuv(s, a)
WFDB_Signal s;
WFDB_Sample a;
{
    double x;
    WFDB_Gain g = (s < nisig) ? isd[s]->info.gain : WFDB_DEFGAIN;

    if (g == 0.) g = WFDB_DEFGAIN;
    x = a*1000./g;
    if (x >= 0.0)
	return ((int)(x + 0.5));
    else
	return ((int)(x - 0.5));
}

FSAMPLE muvadu(s, v)
WFDB_Signal s;
int v;
{
    double x;
    WFDB_Gain g = (s < nisig) ? isd[s]->info.gain : WFDB_DEFGAIN;

    if (g == 0.) g = WFDB_DEFGAIN;
    x = g*v*0.001;
    if (x >= 0.0)
	return ((int)(x + 0.5));
    else
	return ((int)(x - 0.5));
}

FDOUBLE aduphys(s, a)
WFDB_Signal s;
WFDB_Sample a;
{
    int b;
    WFDB_Gain g;

    if (s < nisig) {
	b = isd[s]->info.baseline;
	g = isd[s]->info.gain;
	if (g == 0.) g = WFDB_DEFGAIN;
    }
    else {
	b = 0;
	g = WFDB_DEFGAIN;
    }
    return ((a - b)/g);
}

FSAMPLE physadu(s, v)
WFDB_Signal s;
double v;
{
    int b;
    WFDB_Gain g;

    if (s < nisig) {
	b = isd[s]->info.baseline;
	g = isd[s]->info.gain;
	if (g == 0.) g = WFDB_DEFGAIN;
    }
    else {
	b = 0;
	g = WFDB_DEFGAIN;
    }
    v *= g;
    if (v >= 0)
	return ((int)(v + 0.5) + b);
    else
	return ((int)(v - 0.5) + b);
}

/* sample(s, t) provides buffered random access to the input signals.  The
arguments are the signal number (s) and the sample number (t); the returned
value is the sample from signal s at time t.  On return, the global variable
sample_vflag is true (non-zero) if the returned value is valid, false (zero)
otherwise.  The caller must open the input signals and must set the global
variable nisig to the number of input signals before invoking sample().  Once
this has been done, the caller may request samples in any order. */

#define BUFLN   4096	/* must be a power of 2, see sample() */

FSAMPLE sample(s, t)
WFDB_Signal s;
WFDB_Time t;
{
    static WFDB_Time tt;

    /* Allocate the sample buffer on the first call. */
    if (sbuf == NULL) {
	sbuf= (WFDB_Sample *)malloc((unsigned)nisig*BUFLN*sizeof(WFDB_Sample));
	if (sbuf) tt = (WFDB_Time)-1L;
	else {
	    (void)fprintf(stderr, "sample(): insufficient memory\n");
	    exit(2);
	}
    }
    if (t < 0L) t = 0L;
    /* If the caller has requested a sample that is no longer in the buffer,
       or if the caller has requested a sample that is further ahead than the
       length of the buffer, we need to reset the signal file pointer(s).
       If we do this, we must be sure that the buffer is refilled so that
       any subsequent requests for samples between t - BUFLN+1 and t will
       receive correct responses. */
    if (t <= tt - BUFLN || t > tt + BUFLN) {
	tt = t - BUFLN;
	if (tt < 0L) tt = -1L;
	else if (isigsettime(tt-1) < 0) exit(2);
    }
    /* If the requested sample is not yet in the buffer, read and buffer
       more samples.  If we reach the end of the record, clear sample_vflag
       and return the last valid value. */
    while (t > tt)
        if (getvec(sbuf + nisig * ((++tt)&(BUFLN-1))) < 0) {
	    --tt;
	    sample_vflag = 0;
	    return (*(sbuf + nisig * (tt&(BUFLN-1)) + s));
	}
    /* The requested sample is in the buffer.  Set sample_vflag and
       return the requested sample. */
    sample_vflag = 1;
    return (*(sbuf + nisig * (t&(BUFLN-1)) + s));
}

FINT sample_valid()
{
    return (sample_vflag);
}

/* Private functions (for use by other WFDB library functions only). */

void wfdb_sampquit()
{
    if (sbuf) {
	(void)free(sbuf);
	sbuf = NULL;
	sample_vflag = 0;
    }
}

void wfdb_sigclose()
{
    isigclose();
    osigclose();
    btime = bdate = nsamples = msbtime = msbdate = msnsamples = (WFDB_Time)0;
    sfreq = ifreq = ffreq = (WFDB_Frequency)0;
    pdays = (WFDB_Date)-1;
    segments = in_msrec = skewmax = 0;
    if (dsbuf) {
	(void)free(dsbuf);
	dsbuf = NULL;
	dsbi = -1;
    }
    if (segarray) {
	int i;

	(void)free(segarray);
	segarray = segp = segend = (struct segrec *)NULL;
	for (i = 0; i < maxisig; i++) {
	    if (isd[i]->info.desc) {
		(void)free(isd[i]->info.desc);
		isd[i]->info.desc = NULL;
	    }
	    if (isd[i]->info.units) {
		(void)free(isd[i]->info.units);
		isd[i]->info.units = NULL;
	    }
	}
    }
    if (gv0) (void)free(gv0);
    if (gv1) (void)free(gv1);
    gv0 = gv1 = NULL;
}

void wfdb_osflush()
{
    WFDB_Group g;
    struct ogdata *og;

    for (g = 0; g < nogroups; g++) {
	og = ogd[g];
	if (og->bsize == 0 && og->bp != og->buf) {
	    (void)wfdb_fwrite(og->buf, 1, og->bp - og->buf, og->fp);
	    og->bp = og->buf;
	}
	(void)wfdb_fflush(og->fp);
    }
}
