/* file: signal.c	G. Moody	13 April 1989
			Last revised:  28 January 2022		wfdblib 10.7.0
WFDB library functions for signals

_______________________________________________________________________________
wfdb: a library for reading and writing annotated waveforms (time series data)
Copyright (C) 1989-2013 George B. Moody

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

This file contains definitions of the following functions, which are not
visible outside of this file:
 allocisig	(sets max number of simultaneously open input signals)
 allocigroup	(sets max number of simultaneously open input signal groups)
 allocosig	(sets max number of simultaneously open output signals)
 allocogroup	(sets max number of simultaneously open output signal groups)
 isfmt		(checks if argument is a legal signal format type)
 isflacfmt	(checks if argument refers to a FLAC signal format)
 copysi		(deep-copies a WFDB_Siginfo structure)
 sigmap_cleanup (deallocates memory used by sigmap)
 make_vsd	(makes a virtual signal object)
 sigmap_init	(manages the signal maps)
 sigmap		(creates a virtual signal vector from a raw sample vector)
 edfparse [10.4.5](gets header info from an EDF file)
 readheader	(reads a header file)
 hsdfree	(deallocates memory used by readheader)
 flac_getsamp	(reads the next sample from a FLAC input file)
 flac_isopen	(opens a FLAC input file)
 flac_isclose	(closes a FLAC input file)
 flac_isseek	(skips to a specified location in a FLAC input file)
 flac_putsamp	(writes a sample to a FLAC output file)
 flac_osinit	(prepares to encode a FLAC output file)
 flac_osopen	(opens a FLAC output file)
 flac_osclose	(closes a FLAC output file)
 isigclose	(closes input signals)
 osigclose	(closes output signals)
 isgsetframe	(skips to a specified frame number in a specified signal group)
 getskewedframe	(reads an input frame, without skew correction)
 meansamp       (calculates mean of an array of samples)
 rgetvec        (reads a sample from each input signal without resampling)
 openosig       (opens output signals)

This file also contains low-level I/O routines for signals in various formats;
typically, the input routine for format N signals is named rN(), and the output
routine is named wN().  Most of these routines are implemented as macros for
efficiency.  These low-level I/O routines are not visible outside of this file.

This file also contains definitions of the following WFDB library functions:
 isigopen	(opens input signals)
 osigopen	(opens output signals according to a header file)
 osigfopen	(opens output signals by name)
 findsig [10.4.12] (find an input signal with a specified name)
 getspf [9.6]	(returns number of samples returned by getvec per frame)
 setgvmode [9.0](sets getvec operating mode)
 getgvmode [10.5.3](returns getvec operating mode)
 setifreq [10.2.6](sets the getvec sampling frequency)
 getifreq [10.2.6](returns the getvec sampling frequency)
 getvec		(reads a (possibly resampled) sample from each input signal)
 getframe [9.0]	(reads an input frame)
 putvec		(writes a sample to each output signal)
 isigsettime	(skips to a specified time in each signal)
 isgsettime	(skips to a specified time in a specified signal group)
 tnextvec [10.4.13] (skips to next valid sample of a specified signal)
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
 wfdbputprolog [10.4.15](writes a prolog to a signal file)
 setinfo [10.5.11] (creates a .info file for a record)
 putinfo [4.0]	(writes a line of info for a record)
 getinfo [4.0]	(reads a line of info for a record)
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
 wfdb_sampquit  (frees memory allocated by sample() and sigmap_init())
 wfdb_sigclose 	(closes signals and resets variables)
 wfdb_osflush	(flushes output signals)
 wfdb_freeinfo [10.5.11] (releases resources allocated for info string handling)

Two versions of r16(), r24(), r32(), w16(), w24(), and w32() are provided here.
The default versions are implemented as macros for efficiency.  At least one
C compiler (the Microport System V/AT `cc') is known to run out of space while
attempting to compile r16() and w16().  To avoid this problem, define the
symbol BROKEN_CC when compiling this module, in order to obtain the alternate
versions, which are implemented as functions.

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
void example(void)
{
    int n, nsig, i, framelen;
    WFDB_Siginfo *si;
    WFDB_Sample *vector;

    /* Get the number of signals without opening any signal files. */
    n = isigopen("record", NULL, 0);
    if (n < 1) { /* no signals -- quit or try another record, etc. */ }
	
    /* Allocate WFDB_Siginfo structures before calling isigopen again. */
    SUALLOC(si, n, sizeof(WFDB_Siginfo));
    nsig = isigopen("record", si, n);
    /* Note that nsig equals n only if all signals were readable. */

    /* Get the number of samples per frame. */
    for (i = framelen = 0; i < nsig; i++)
        framelen += si[i].spf;
    /* Allocate WFDB_Samples before calling getframe. */
    SUALLOC(vector, framelen, sizeof(WFDB_Sample));
    getframe(vector);
}
#endif

#include "wfdblib.h"

#include <limits.h>
#include <errno.h>

#ifdef iAPX286
#define BROKEN_CC
#endif

#ifndef NOTIME
#include <time.h>
#endif

#ifdef WFDB_FLAC_SUPPORT
#include <FLAC/stream_encoder.h>
#include <FLAC/stream_decoder.h>
#else
#define FLAC__StreamEncoder struct dummy
#define FLAC__StreamDecoder struct dummy
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
static int spfmax;		/* max number of samples per frame */
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
static WFDB_Seginfo *segarray, *segp, *segend;
				/* beginning, current segment, end pointers */
static struct WFDB_seginfo_L *segarray_L;

/* These variables relate to open input signals. */
static unsigned maxisig;	/* max number of input signals */
static unsigned maxigroup;	/* max number of input signal groups */
static unsigned nisig;		/* number of open input signals */
static unsigned nigroup;	/* number of open input signal groups */
static unsigned ispfmax;	/* max number of samples of any open signal
				   per input frame */
static struct isdata {		/* unique for each input signal */
    WFDB_Siginfo info;		/* input signal information */
    WFDB_Sample samp;		/* most recent sample read */
    int skew;			/* intersignal skew (in frames) */
    int gvindex;		/* current high-resolution sample number */
    int gvcount;		/* counter for updating gvindex */
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
    FLAC__StreamDecoder *flacdec; /* internal state for FLAC decoder */
    char *packptr;		/* pointer to next partially-decoded frame */
    unsigned packspf;		/* number of samples per signal per frame */
    unsigned packcount; 	/* number of samples decoded in this frame */
    char count;			/* input counter for bit-packed signal */
    char seek;			/* 0: do not seek on file, 1: seeks permitted */
    char initial_skip;		/* 1 if isgsetframe is needed before reading */
    int stat;			/* signal file status flag */
} **igd;
static WFDB_Sample *tvector;	/* getvec workspace */
static WFDB_Sample *uvector;	/* isgsettime workspace */
static WFDB_Sample *vvector;	/* tnextvec workspace */
static int tuvlen;		/* lengths of tvector and uvector in samples */
static WFDB_Time istime;	/* time of next input sample */
static int ibsize;		/* default input buffer size */
static unsigned skewmax;	/* max skew (frames) between any 2 signals */
static WFDB_Sample *dsbuf;	/* deskewing buffer */
static int dsbi;		/* index to oldest sample in dsbuf (if < 0,
				   dsbuf does not contain valid data) */
static unsigned dsblen;		/* capacity of dsbuf, in samples */
static unsigned framelen;	/* total number of samples per frame */
static int gvmode = DEFWFDBGVMODE;	/* getvec mode */
static int gvc;			/* getvec sample-within-frame counter */
static int isedf;		/* if non-zero, record is stored as EDF/EDF+ */
static WFDB_Sample *sbuf = NULL;	/* buffer used by sample() */
static int sample_vflag;	/* if non-zero, last value returned by sample()
				   was valid */

/* These variables relate to output signals. */
static unsigned maxosig;	/* max number of output signals */
static unsigned maxogroup;	/* max number of output signal groups */
static unsigned nosig;		/* number of open output signals */
static unsigned nogroup;	/* number of open output signal groups */
static WFDB_FILE *oheader;	/* file pointer for output header file */
static WFDB_FILE *outinfo;	/* file pointer for output info file */
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
    FLAC__StreamEncoder *flacenc; /* internal state for FLAC encoder */
    unsigned packspf;		/* number of samples per frame */
    char count;		/* output counter for bit-packed signal */
    signed char seek;		/* 1: seek works, -1: seek doesn't work,
				   0: unknown */
    char force_flush;		/* flush even if seek doesn't work */
    char nrewind;		/* number of bytes to seek backwards
				   after flushing */
} **ogd;
static WFDB_Time ostime;	/* time of next output sample */
static int obsize;		/* default output buffer size */

/* These variables relate to info strings. */
static char **pinfo;	/* array of info string pointers */
static int nimax;	/* number of info string pointers allocated */
static int ninfo;	/* number of info strings read */

/* Local functions (not accessible outside this file). */

static char *ftimstr(WFDB_Time t, WFDB_Frequency f);
static char *fmstimstr(WFDB_Time t, WFDB_Frequency f);
static WFDB_Time fstrtim(const char *string, WFDB_Frequency f);

/* Allocate workspace for up to n input signals. */
static int allocisig(unsigned int n)
{
    if (maxisig < n) {
	unsigned m = maxisig;

	SREALLOC(isd, n, sizeof(struct isdata *));
	while (m < n) {
	    SUALLOC(isd[m], 1, sizeof(struct isdata));
	    m++;
	}
	maxisig = n;
    }
    return (maxisig);
}

/* Allocate workspace for up to n input signal groups. */
static int allocigroup(unsigned int n)
{
    if (maxigroup < n) {
	unsigned m = maxigroup;

	SREALLOC(igd, n, sizeof(struct igdata *));
	while (m < n) {
	    SUALLOC(igd[m], 1, sizeof(struct igdata));
	    m++;
	}
	maxigroup = n;
    }
    return (maxigroup);
}

/* Allocate workspace for up to n output signals. */
static int allocosig(unsigned int n)
{
    if (maxosig < n) {
	unsigned m = maxosig;

	SREALLOC(osd, n, sizeof(struct osdata *));
	while (m < n) {
	    SUALLOC(osd[m], 1, sizeof(struct osdata));
	    m++;
	}
	maxosig = n;
    }
    return (maxosig);
}

/* Allocate workspace for up to n output signal groups. */
static int allocogroup(unsigned int n)
{
    if (maxogroup < n) {
	unsigned m = maxogroup;

	SREALLOC(ogd, n, sizeof(struct ogdata *));
	while (m < n) {
	    SUALLOC(ogd[m], 1, sizeof(struct ogdata));
	    m++;
	}
	maxogroup = n;
    }
    return (maxogroup);
}

static int isfmt(int f)
{
    int i;
    static int fmt_list[WFDB_NFMTS] = WFDB_FMT_LIST;

    for (i = 0; i < WFDB_NFMTS; i++)
	if (f == fmt_list[i]) return (1);
    return (0);
}

static int isflacfmt(int f)
{
    return (f >= 500 && f <= 532);
}

static int copysi(WFDB_Siginfo *to, const WFDB_Siginfo *from)
{
    if (to == NULL || from == NULL) return (0);
    *to = *from;
    to->fname = to->desc = to->units = NULL;
    SSTRCPY(to->fname, from->fname);
    SSTRCPY(to->desc, from->desc);
    SSTRCPY(to->units, from->units);
    return (1);
}

#ifndef WFDB_LARGETIME
# define strtotime strtol
#else
# define strtotime strtoll
#endif

/* Code for handling variable-layout multi-segment records

    The following code (together with minor changes elsewhere in this file)
   was written to permit reading a record consisting of multiple segments
   on-the-fly, even if the segments do not contain the same signals or the
   same number of signals.  If the gain or baseline of any signal changes
   from segment to segment, the code in this section adjusts for the changes
   transparently, so that a WFDB application can assume that all signals
   are always present in the same order, with constant gains and baselines
   as specified in the .hea file for the first segment.  This .hea file
   is in the same format as an ordinary .hea file, but if its length is
   specified as zero samples, it is recognized as a "layout header".  In
   this case, the signal file name is conventionally given as "~" (although
   any name is acceptable), and the format is given as 0 (required).

   The "layout header" contains a signal description line for each signal
   of interest that appears anywhere in the record.  The gain and baseline
   specified in the layout header are those that will apply to samples
   throughout the record (sigmap, below, scales and shifts the raw samples
   as needed).

   If a gap occurs between the end of one segment and the beginning of the
   next, a special "null" segment can be listed in the master .hea file,
   like this:
       ~ 4590
   The segment name, "~", does not correspond to a real .hea file, but the
   number that follows indicates the length of the gap in sample intervals.
 */

static int need_sigmap, maxvsig, nvsig, tspf, vspfmax;
static struct isdata **vsd;
static WFDB_Sample *ovec;

static struct sigmapinfo {
    char *desc;
    double gain, scale, offset;
    WFDB_Sample sample_offset;
    WFDB_Sample baseline;
    int index;
    int spf;
} *smi;

static void sigmap_cleanup(void)
{
    int i;

    need_sigmap = nvsig = tspf = vspfmax = 0;
    SFREE(ovec);
    if (smi) {
	for (i = 0; i < tspf; i += smi[i].spf)
	    SFREE(smi[i].desc);
	SFREE(smi);
    }

    if (vsd) {
	struct isdata *is;

	while (maxvsig)
	    if (is = vsd[--maxvsig]) {
		SFREE(is->info.fname);
		SFREE(is->info.units);
		SFREE(is->info.desc);
		SFREE(is);
	    }
    	SFREE(vsd);
    }
}

static int make_vsd(void)
{
    int i;

    if (nvsig != nisig) {
	wfdb_error("make_vsd: oops! nvsig = %d, nisig = %d\n", nvsig, nisig);
	return (-1);
    }
    for (i = 0; i < maxvsig; i++) {
        SFREE(vsd[i]->info.fname);
        SFREE(vsd[i]->info.desc);
        SFREE(vsd[i]->info.units);
    } 
    if (maxvsig < nvsig) {
	unsigned m = maxvsig;

	SREALLOC(vsd, nvsig, sizeof(struct isdata *));
	while (m < nvsig) {
	    SUALLOC(vsd[m], 1, sizeof(struct isdata));
	    m++;
	}
	maxvsig = nvsig;
    }

    for (i = 0; i < nvsig; i++) {
	copysi(&vsd[i]->info, &isd[i]->info);
	vsd[i]->skew = isd[i]->skew;
    }

    return (nvsig);
}

static int sigmap_init(int first_segment)
{
    int i, j, k, kmax, s, ivmin, ivmax;
    double ovmin, ovmax;
    struct sigmapinfo *ps;

    /* is this the layout segment?  if so, set up output side of map */
    if (in_msrec && first_segment && segarray && segarray[0].nsamp == 0) {
	need_sigmap = 1;

	/* The number of virtual signals is the number of signals defined
	   in the layout segment. */
	nvsig = nisig;
	vspfmax = ispfmax;
	for (s = tspf = 0; s < nisig; s++)
	    tspf += isd[s]->info.spf;
	SALLOC(smi, tspf, sizeof(struct sigmapinfo));
	for (i = s = 0; i < nisig; i++) {
	    SSTRCPY(smi[s].desc, isd[i]->info.desc);
	    smi[s].gain = isd[i]->info.gain;
	    smi[s].baseline = isd[i]->info.baseline;
	    k = smi[s].spf = isd[i]->info.spf;
	    for (j = 1; j < k; j++)
		smi[s + j] = smi[s];
	    s += k;	    
	}
	SALLOC(ovec, tspf, sizeof(WFDB_Sample));
	return (make_vsd());
    }

    else if (need_sigmap) {	/* set up the input side of the map */
	for (s = 0; s < tspf; s++) {
	    smi[s].index = 0;
	    smi[s].scale = 0.;
	    smi[s].offset = 0.;
	    smi[s].sample_offset = WFDB_INVALID_SAMPLE;
	}
	ispfmax = vspfmax;

	if (isd[0]->info.fmt == 0 && nisig == 1)
	    return (0);    /* the current segment is a null record */

	for (i = j = 0; i < nisig; j += isd[i++]->info.spf)
	    for (s = 0; s < tspf; s += smi[s].spf)
		if (strcmp(smi[s].desc, isd[i]->info.desc) == 0) {
		    if ((kmax = smi[s].spf) != isd[i]->info.spf) {
			wfdb_error(
		   "sigmap_init: unexpected spf for signal %d in segment %s\n",
		                   i, segp->recname);
			if (kmax > isd[i]->info.spf)
			    kmax = isd[i]->info.spf;
		    }
		    for (k = 0; k < kmax; k++) {
			ps = &smi[s + k];
			ps->index = j + k;
			ps->scale = ps->gain / isd[i]->info.gain;
			if (ps->scale < 1.0)
			    wfdb_error(
	       "sigmap_init: loss of precision in signal %d in segment %s\n",
				       i, segp->recname);
			ps->offset = ps->baseline -
			             ps->scale * isd[i]->info.baseline + 0.5;

			/* If it is possible to add an additional
			   offset such that all possible output values
			   will fit into a positive signed integer, we
			   can use the "fast" case in sigmap, below. */
			switch (isd[i]->info.fmt) {
			  case 508:
			  case 80: ivmin = -0x80; ivmax = 0x7f; break;
			  case 310:
			  case 311: ivmin = -0x200; ivmax = 0x1ff; break;
			  case 212: ivmin = -0x800; ivmax = 0x7ff; break;
			  case 16:
			  case 61:
			  case 516:
			  case 160: ivmin = -0x8000; ivmax = 0x7fff; break;
			  case 524:
			  case 24: ivmin = -0x800000; ivmax = 0x7fffff; break;
			  default:
			    ivmin = WFDB_SAMPLE_MIN;
			    ivmax = WFDB_SAMPLE_MAX;
			    break;
			}
			ovmin = ivmin * ps->scale + ps->offset;
			ovmax = ivmax * ps->scale + ps->offset;
			if (ovmin < ovmax &&
			    ovmin >= WFDB_SAMPLE_MIN + 1 &&
			    ovmax <= WFDB_SAMPLE_MAX &&
			    ovmax - ovmin + 1 < WFDB_SAMPLE_MAX) {
			    ps->sample_offset = ovmin - 1;
			    ps->offset -= ps->sample_offset;
			}
			else {
			    ps->sample_offset = 0;
			}
		    }
		    break;
		}
	if (j > tspf) {
	    wfdb_error("sigmap_init: frame size too large in segment %s\n",
		       segp->recname);
	    return (-1);
	}
    }

    else if (in_msrec && !first_segment && framelen == 0) {
	/* opening a new segment of a fixed-layout multisegment
	   record */
	ispfmax = vspfmax;
	if (nisig > nvsig) {
	    wfdb_error("sigmap_init: wrong number of signals in segment %s\n",
		       segp->recname);
	    return (-1);
	}
	for (i = 0; i < nisig; i++) {
	    if (isd[i]->info.spf != vsd[i]->info.spf) {
		wfdb_error(
		    "sigmap_init: wrong spf for signal %d in segment %s\n",
		    i, segp->recname);
		return (-1);
	    }
	}
    }

    else {	/* normal record, or multisegment record without a dummy
		   header */
	nvsig = nisig;
	vspfmax = ispfmax;
	for (s = tspf = 0; s < nisig; s++)
	    tspf += isd[s]->info.spf;
	return (make_vsd());
    }

    return (0);
}

static int sigmap(WFDB_Sample *vector, const WFDB_Sample *ivec)
{
    int i;
    double v;

    for (i = 0; i < tspf; i++) {
	if (ivec[smi[i].index] == WFDB_INVALID_SAMPLE)
	    vector[i] = WFDB_INVALID_SAMPLE;
	else {
	    /* Scale the input sample and round it to the nearest
	       integer.  Halfway cases are always rounded up (10.5 is
	       rounded to 11, but -10.5 is rounded to -10.)  Note that
	       smi[i].offset already includes an extra 0.5, so we
	       simply need to calculate the floor of v. */
	    v = ivec[smi[i].index] * smi[i].scale + smi[i].offset;
	    if (smi[i].sample_offset) {
		/* Fast case: if we can guarantee that v is always
		   positive and the following calculation cannot
		   overflow, we can avoid additional floating-point
		   operations. */
		vector[i] = (WFDB_Sample) v + smi[i].sample_offset;
	    }
	    else {
		/* Slow case: we need to check bounds and handle
		   negative values. */
		if (v >= 0) {
		    if (v <= WFDB_SAMPLE_MAX)
			vector[i] = (WFDB_Sample) v;
		    else
			vector[i] = WFDB_SAMPLE_MAX;
		}
		else {
		    if (v >= WFDB_SAMPLE_MIN) {
			vector[i] = (WFDB_Sample) v;
			if (vector[i] > v)
			    vector[i]--;
		    }
		    else {
			vector[i] = WFDB_SAMPLE_MIN;
		    }
		}
	    }
	}
    }
    return (tspf);
}

/* end of code for handling variable-layout records */

/* Read a fixed-size character field and remove trailing spaces. */
static void read_edf_str(char *buf, int size, WFDB_FILE *ifile)
{
    if (wfdb_fread(buf, 1, size, ifile) != size) {
	*buf = 0;
    }
    else {
	while (size > 0 && buf[size - 1] == ' ')
	    size--;
	buf[size] = 0;
    }
}

/* get header information from an EDF file */
static int edfparse(WFDB_FILE *ifile)
{
    char buf[81], *edf_fname, *p;
    double *pmax, *pmin, spr, baseline;
    int format, i, s, nsig, offset, day, month, year, hour, minute, second;
    long adcrange, *dmax, *dmin, nframes;
 
    edf_fname = wfdbfile(NULL, NULL);

    /* Read the first 8 bytes and check for the magic string.  (This might
       accept some non-EDF files.) */
    wfdb_fread(buf, 1, 8, ifile);
    if (strncmp(buf, "0       ", 8) == 0)
	format = 16;	/* EDF or EDF+ */
    else if (strncmp(buf+1, "BIOSEMI", 7) == 0)
	format = 24;	/* BDF */
    else {
	wfdb_error("init: '%s' is not EDF or EDF+\n", edf_fname);
	return (-2);
    }

    /* Read the remainder of the fixed-size section of the header. */
    read_edf_str(buf, 80, ifile);	/* patient ID (ignored) */
    read_edf_str(buf, 80, ifile);	/* recording ID (ignored) */
    read_edf_str(buf, 8, ifile);	/* recording date */
    sscanf(buf, "%d%*c%d%*c%d", &day, &month, &year);
    year += 1900;			/* EDF has only two-digit years */
    if (year < 1985) year += 100;	/* fix this before 1/1/2085! */
    read_edf_str(buf, 8, ifile);	/* recording time */
    sscanf(buf, "%d%*c%d%*c%d", &hour, &minute, &second);
    read_edf_str(buf, 8, ifile);	/* number of bytes in header */
    sscanf(buf, "%d", &offset);
    read_edf_str(buf, 44, ifile);	/* free space (ignored) */
    read_edf_str(buf, 8, ifile);	/* number of frames (EDF blocks) */
    sscanf(buf, "%ld", &nframes);
    if (nframes < 0) nframes = 0;
    nsamples = nframes;
    read_edf_str(buf, 8, ifile);	/* data record duration (seconds) */
    sscanf(buf, "%lf", &spr);
    if (spr <= 0.0) spr = 1.0;
    wfdb_fread(buf+4, 1, 4, ifile);	/* number of signals */
    sscanf(buf+4, "%d", &nsig);

    if (nsig < 1 || (nsig + 1)*256 != offset) {
	wfdb_error("init: '%s' is not EDF or EDF+\n", edf_fname);
	return (-2);
    }

    /* Allocate workspace. */
    if (maxhsig < nsig) {
	unsigned m = maxhsig;

	SREALLOC(hsd, nsig, sizeof(struct hsdata *));
	while (m < nsig) {
	    SUALLOC(hsd[m], 1, sizeof(struct hsdata));
	    m++;
	}
	maxhsig = nsig;
    }
    SUALLOC(dmax, nsig, sizeof(long));
    SUALLOC(dmin, nsig, sizeof(long));
    SUALLOC(pmax, nsig, sizeof(double));
    SUALLOC(pmin, nsig, sizeof(double));

    /* Strip off any path info from the EDF file name. */
    p = edf_fname + strlen(edf_fname) - 4;
    while (--p > edf_fname)
	if (*p == '/') edf_fname = p+1;

    /* Read the variable-size section of the header. */
    for (s = 0; s < nsig; s++) {
	hsd[s]->start = offset;
	hsd[s]->skew = 0;
	SSTRCPY(hsd[s]->info.fname, edf_fname);
	hsd[s]->info.group = hsd[s]->info.bsize = hsd[s]->info.cksum = 0;
	hsd[s]->info.fmt = format;
	hsd[s]->info.nsamp = nframes;

	read_edf_str(buf, 16, ifile);	/* signal type */
	SSTRCPY(hsd[s]->info.desc, buf);
    }

    for (s = 0; s < nsig; s++)
	read_edf_str(buf, 80, ifile); /* transducer type (ignored) */

    for (s = 0; s < nsig; s++) {
	read_edf_str(buf, 8, ifile);	/* signal units */
	SSTRCPY(hsd[s]->info.units, buf);
    }

    for (s = 0; s < nsig; s++) {
	read_edf_str(buf, 8, ifile);	/* physical minimum */
	sscanf(buf, "%lf", &pmin[s]);
    }

    for (s = 0; s < nsig; s++) {
	read_edf_str(buf, 8, ifile);	/* physical maximum */
	sscanf(buf, "%lf", &pmax[s]);
    }

    for (s = 0; s < nsig; s++) {
	read_edf_str(buf, 8, ifile);	/* digital minimum */
	sscanf(buf, "%ld", &dmin[s]);
    }

    for (s = 0; s < nsig; s++) {
	read_edf_str(buf, 8, ifile);	/* digital maximum */
	sscanf(buf, "%ld", &dmax[s]);
	hsd[s]->info.initval = hsd[s]->info.adczero = (dmax[s]+1 + dmin[s])/2;
	adcrange = dmax[s] - dmin[s];
	for (i = 0; adcrange > 0; i++)
	    adcrange /= 2;
	hsd[s]->info.adcres = i;
	if (pmax[s] != pmin[s]) {
	    hsd[s]->info.gain = (dmax[s] - dmin[s])/(pmax[s] - pmin[s]);
	    baseline = dmax[s] - pmax[s] * hsd[s]->info.gain;
	    if (baseline >= 0.0)
		hsd[s]->info.baseline = baseline + 0.5;
	    else
		hsd[s]->info.baseline = baseline - 0.5;
	}
	else			/* gain is undefined */
	    hsd[s]->info.gain = hsd[s]->info.baseline = 0;
    }

    for (s = 0; s < nsig; s++)
	read_edf_str(buf, 80, ifile);	/* filtering information (ignored) */

    for (s = 0; s < nsig; s++) {
	int n;

	read_edf_str(buf, 8, ifile);	/* samples per frame (EDF block) */
	sscanf(buf, "%d", &n);
	if ((hsd[s]->info.spf = n) > spfmax) spfmax = n;
    }

    (void)wfdb_fclose(ifile);	/* (don't bother reading nsig*32 bytes of free
				   space) */
    hheader = NULL;	/* make sure getinfo doesn't try to read the EDF file */

    ffreq = 1.0 / spr;	/* frame frequency = 1/(seconds per EDF block) */
    cfreq = ffreq; /* set sampling and counter frequencies to match */
    sfreq = ffreq * spfmax;
    if (getafreq() == 0.0) setafreq(sfreq);
    gvmode |= WFDB_HIGHRES;
    sprintf(buf, "%02d:%02d:%02d %02d/%02d/%04d",
	    hour, minute, second, day, month, year);
    setbasetime(buf);

    SFREE(pmin);
    SFREE(pmax);
    SFREE(dmin);
    SFREE(dmax);
    isedf = 1;
    return (nsig);
}

static int readheader(const char *record)
{
    char linebuf[256], *p, *q;
    WFDB_Frequency f;
    WFDB_Signal s;
    WFDB_Time ns;
    unsigned int i, nsig;
    static char sep[] = " \t\n\r";

    /* If another input header file was opened, close it. */
    if (hheader) {
	(void)wfdb_fclose(hheader);
	hheader = NULL;
    }

    spfmax = 1;
    sfreq = ffreq;
    isedf = 0;
    if (strcmp(record, "~") == 0) {
	if (in_msrec && vsd) {
	    char *p;

	    SALLOC(hsd, 1, sizeof(struct hsdata *));
	    SALLOC(hsd[0], 1, sizeof(struct hsdata));
	    SSTRCPY(hsd[0]->info.desc, "~");
	    hsd[0]->info.spf = 1;
	    hsd[0]->info.fmt = 0;
	    hsd[0]->info.nsamp = nsamples = segp->nsamp;
	    return (maxhsig = 1);	       
	}
	return (0);
    }

    /* If the final component of the record name includes a '.', assume it is a
       file name. */
    q = (char *)record + strlen(record) - 1;
    while (q > record && *q != '.' && *q != '/' && *q != ':' && *q != '\\')
	q--;
    if (*q == '.') {
	if ((hheader = wfdb_open(NULL, record, WFDB_READ)) == NULL) {
	    wfdb_error("init: can't open %s\n", record);
	    return (-1);
	}
	else if (strcmp(q+1, "hea"))	/* assume EDF if suffix is not '.hea' */
	    return (edfparse(hheader));
    }

    /* Otherwise, assume the file name is record.hea. */
    else if ((hheader = wfdb_open("hea", record, WFDB_READ)) == NULL) {
	wfdb_error("init: can't open header for record %s\n", record);
	return (-1);
    }

    /* Read the first line and check for a magic string. */
    if (wfdb_fgets(linebuf, 256, hheader) == NULL) {
        wfdb_error("init: record %s header is empty\n", record);
	    return (-2);
    }
    if (strncmp("#wfdb", linebuf, 5) == 0) { /* found the magic string */
	int i, major, minor = 0, release = 0;

	i = sscanf(linebuf+5, "%d.%d.%d", &major, &minor, &release);
	if ((i > 0 && major > WFDB_MAJOR) ||
	    (i > 1 && major == WFDB_MAJOR && minor > WFDB_MINOR) ||
	    (i > 2 && major == WFDB_MAJOR && minor == WFDB_MINOR &&
	     release > WFDB_RELEASE)) {
	    wfdb_error("init: reading record %s requires WFDB library "
		       "version %d.%d.%d or later\n"
"  (the most recent version is always available from http://physionet.org)\n",
		       record, major, minor, release);
	    return (-1);
	}
    }

    /* Get the first token (the record name) from the first non-empty,
       non-comment line. */
    while ((p = strtok(linebuf, sep)) == NULL || *p == '#') {
	if (wfdb_fgets(linebuf, 256, hheader) == NULL) {
	    wfdb_error("init: can't find record name in record %s header\n",
		     record);
	    return (-2);
	}
    }

    for (q = p+1; *q && *q != '/'; q++)
	;
    if (*q == '/') {
	if (in_msrec) {
	    wfdb_error(
	  "init: record %s cannot be nested in another multi-segment record\n",
		     record);
	    return (-2);
	}
	segments = strtol(q+1, NULL, 10);
	*q = '\0';
    }

    /* For local files, be sure that the name (p) within the header file
       matches the name (record) provided as an argument to this function --
       if not, the header file may have been renamed in error or its contents
       may be corrupted.  The requirement for a match is waived for remote
       files since the user may not be able to make any corrections to them. */
    if (hheader->type == WFDB_LOCAL &&
	hheader->fp != stdin && strncmp(p, record, strlen(p)) != 0) {
	/* If there is a mismatch, check to see if the record argument includes
	   a directory separator (whether valid or not for this OS);  if so,
	   compare only the final portion of the argument against the name in
	   the header file. */
	const char *q, *r, *s;

	for (r = record, q = s = r + strlen(r) - 1; r != s; s--)
	    if (*s == '/' || *s == '\\' || *s == ':')
		break;

	if (q > s && (r > s || strcmp(p, s+1) != 0)) {
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
    nsig = (unsigned)strtol(p, NULL, 10);

    /* Determine the frame rate, if present and not set already. */
    if (p = strtok((char *)NULL, sep)) {
	if ((f = (WFDB_Frequency)strtod(p, NULL)) <= (WFDB_Frequency)0.) {
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
	    cfreq = strtod(++p, NULL);
	    for ( ; *p && *p != '('; p++)
		;
	    if (*p == '(')
		bcount = strtod(++p, NULL);
	}
    }
    if (cfreq <= 0.0) cfreq = ffreq;

    /* Determine the number of samples per signal, if present and not
       set already. */
    if (p = strtok((char *)NULL, sep)) {
	if ((ns = strtotime(p, NULL, 10)) < 0L) {
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
	SALLOC(segarray, segments, sizeof(WFDB_Seginfo));
	SFREE(segarray_L);
	segp = segarray;
	for (i = 0, ns = (WFDB_Time)0L; i < segments; i++, segp++) {
	    /* Get next segment spec, skip empty lines and comments. */
	    do {
		if (wfdb_fgets(linebuf, 256, hheader) == NULL) {
		    wfdb_error(
			"init: unexpected EOF in header file for record %s\n",
			record);
		    SFREE(segarray);
		    segments = 0;
		    return (-2);
		}
	    } while ((p = strtok(linebuf, sep)) == NULL || *p == '#');
	    if (*p == '+') {
		wfdb_error(
		    "init: `%s' is not a valid segment name in record %s\n",
		    p, record);
		SFREE(segarray);
		segments = 0;
		return (-2);
	    }
	    if (strlen(p) > WFDB_MAXRNL) {
		wfdb_error(
		    "init: `%s' is too long for a segment name in record %s\n",
		    p, record);
		SFREE(segarray);
		segments = 0;
		return (-2);
	    }
	    (void)strcpy(segp->recname, p);
	    if ((p = strtok((char *)NULL, sep)) == NULL ||
		(segp->nsamp = strtotime(p, NULL, 10)) < 0L) {
		wfdb_error(
		"init: length must be specified for segment %s in record %s\n",
		           segp->recname, record);
		SFREE(segarray);
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
	    wfdb_error("warning (init): in record %s, "
		       "stated record length (%"WFDB_Pd_TIME")\n",
		       record, msnsamples);
	    wfdb_error(" does not match sum of segment lengths "
		       "(%"WFDB_Pd_TIME")\n", ns);
	}
	return (0);
    }

    /* Allocate workspace. */
    if (maxhsig < nsig) {
	unsigned m = maxhsig;

	SREALLOC(hsd, nsig, sizeof(struct hsdata *));
	while (m < nsig) {
	    SUALLOC(hsd[m], 1, sizeof(struct hsdata));
	    m++;
	}
	maxhsig = nsig;
    }

    /* Now get information for each signal. */
    for (s = 0; s < nsig; s++) {
	struct hsdata *hp, *hs;
	int nobaseline;

	hs = hsd[s];
	hp = (s ? hsd[s-1] : NULL);
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
	    SSTRCPY(hs->info.fname, p);
	}
	/* If the file names of the current and previous signals match,
	   they are assigned the same group number and share a copy of the
	   file name.  All signals associated with a given file must be
	   listed together in the header in order to be identified as
	   belonging to the same group;  readheader does not check that
	   this has been done. */
	else {
	    hs->info.group = hp->info.group;
	    SSTRCPY(hs->info.fname, hp->info.fname);
	}

	/* Determine the signal format. */
	if ((p = strtok((char *)NULL, sep)) == NULL ||
	    !isfmt(hs->info.fmt = strtol(p, NULL, 10))) {
	    wfdb_error("init: illegal format for signal %d, record %s\n",
		       s, record);
	    return (-2);
	}
	hs->info.spf = 1;
	hs->skew = 0;
	hs->start = 0L;
	while (*(++p)) {
	    if (*p == 'x' && *(++p))
		if ((hs->info.spf = strtol(p, NULL, 10)) < 1) hs->info.spf = 1;
	    if (*p == ':' && *(++p))
		if ((hs->skew = strtol(p, NULL, 10)) < 0) hs->skew = 0;
	    if (*p == '+' && *(++p))
		if ((hs->start = strtol(p, NULL, 10)) < 0L) hs->start = 0L;
	}
	if (hs->info.spf > spfmax) spfmax = hs->info.spf;
	/* The resolution for deskewing is one frame.  The skew in samples
	   (given in the header) is converted to skew in frames here. */
	hs->skew = (int)(((double)hs->skew)/hs->info.spf + 0.5);

	/* Determine the gain in ADC units per physical unit.  This number
	   may be zero or missing;  if so, the signal is uncalibrated. */
	if (p = strtok((char *)NULL, sep))
	    hs->info.gain = (WFDB_Gain)strtod(p, NULL);
	else
	    hs->info.gain = (WFDB_Gain)0.;

	/* Determine the baseline if specified, and the physical units
	   (assumed to be millivolts unless otherwise specified). */
	nobaseline = 1;
	if (p) {
	    for ( ; *p && *p != '(' && *p != '/'; p++)
		;
	    if (*p == '(') {
		hs->info.baseline = strtol(++p, NULL, 10);
		nobaseline = 0;
	    }
	    while (*p)
		if (*p++ == '/' && *p)
		    break;
	}
	if (p && *p) {
	    SALLOC(hs->info.units, WFDB_MAXUSL+1, 1);
	    (void)strncpy(hs->info.units, p, WFDB_MAXUSL);
	}
	else
	    hs->info.units = NULL;

	/* Determine the ADC resolution in bits.  If this number is
	   missing and cannot be inferred from the format, the default
	   value (from wfdb.h) is filled in. */
	if (p = strtok((char *)NULL, sep))
	    i = (unsigned)strtol(p, NULL, 10);
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
	hs->info.adczero = (p=strtok((char *)NULL,sep)) ? strtol(p,NULL,10) : 0;
	    
	/* Set the baseline to adczero if no baseline field was found. */
	if (nobaseline) hs->info.baseline = hs->info.adczero;

	/* Determine the initial value (assumed to be equal to the ADC 
	   zero if missing). */
	hs->info.initval = (p = strtok((char *)NULL, sep)) ?
	    strtol(p, NULL, 10) : hs->info.adczero;

	/* Determine the checksum (assumed to be zero if missing). */
	if (p = strtok((char *)NULL, sep)) {
	    hs->info.cksum = strtol(p, NULL, 10);
	    hs->info.nsamp = (ns > LONG_MAX ? 0 : ns);
	}
	else {
	    hs->info.cksum = 0;
	    hs->info.nsamp = (WFDB_Time)0L;
	}

	/* Determine the block size (assumed to be zero if missing). */
	hs->info.bsize = (p = strtok((char *)NULL,sep)) ? strtol(p,NULL,10) : 0;

	/* Check that formats and block sizes match for signals belonging
	   to the same group. */
	if (s && (hp == NULL || (hs->info.group == hp->info.group &&
	    (hs->info.fmt != hp->info.fmt ||
	     hs->info.bsize != hp->info.bsize)))) {
	    wfdb_error("init: error in specification of signal %d or %d\n",
		       s-1, s);
	    return (-2);
	}
	    
	/* Get the signal description.  If missing, a description of
	   the form "record xx, signal n" is filled in. */
	SALLOC(hs->info.desc, 1, WFDB_MAXDSL+1);
	if (p = strtok((char *)NULL, "\n\r"))
	    (void)strncpy(hs->info.desc, p, WFDB_MAXDSL);
	else
	    (void)sprintf(hs->info.desc,
			  "record %s, signal %d", record, s);
    }
    setgvmode(gvmode);		/* Reset sfreq if appropriate. */
    return (s);			/* return number of available signals */
}

static void hsdfree(void)
{
    struct hsdata *hs;

    if (hsd) {
	while (maxhsig)
	    if (hs = hsd[--maxhsig]) {
		SFREE(hs->info.fname);
		SFREE(hs->info.units);
		SFREE(hs->info.desc);
		SFREE(hs);
	    }
	SFREE(hsd);
    }
    maxhsig = 0;
}

/* Routines for reading FLAC signal files.

   Terminology note: the FLAC format refers to an encoded block of
   samples as a "frame".  In WFDB terminology, a "frame" is a group of
   samples representing a fixed time interval (specified by the frame
   frequency in the header file.)  A FLAC frame may represent the
   contents of multiple WFDB frames, or only part of one.  Unless
   otherwise noted, the word "frame" refers to a WFDB frame. */

#ifdef WFDB_FLAC_SUPPORT

/* The following functions (iflac_read, iflac_seek, iflac_tell,
   iflac_length, iflac_eof, iflac_error, and iflac_samples) are used
   as callbacks for the FLAC library.  The client_data argument is the
   value passed to FLAC__stream_decoder_init_stream, which is a
   pointer to a struct igdata. */

/* iflac_read is called by the FLAC library in order to read a chunk of
   data from the input file. */
static FLAC__StreamDecoderReadStatus
iflac_read(const FLAC__StreamDecoder *dec, FLAC__byte buffer[],
	   size_t *bytes, void *client_data)
{
    struct igdata *g = client_data;
    if (*bytes == 0)
	return (FLAC__STREAM_DECODER_READ_STATUS_ABORT);
    *bytes = wfdb_fread(buffer, sizeof(FLAC__byte), *bytes, g->fp);
    if (wfdb_ferror(g->fp))
	return (FLAC__STREAM_DECODER_READ_STATUS_ABORT);
    else if (*bytes == 0)
	return (FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM);
    else
	return (FLAC__STREAM_DECODER_READ_STATUS_CONTINUE);
}

/* iflac_seek is called by the FLAC library in order to move to a
   different position in the input file. */
static FLAC__StreamDecoderSeekStatus
iflac_seek(const FLAC__StreamDecoder *dec, FLAC__uint64 pos,
	   void *client_data)
{
    struct igdata *g = client_data;
    if (!g->seek)
	return (FLAC__STREAM_DECODER_SEEK_STATUS_UNSUPPORTED);
    else if (pos > LONG_MAX || wfdb_fseek(g->fp, pos, SEEK_SET))
	return (FLAC__STREAM_DECODER_SEEK_STATUS_ERROR);
    else
	return (FLAC__STREAM_DECODER_SEEK_STATUS_OK);
}

/* iflac_tell is called by the FLAC library in order to determine the
   current position in the input file. */
static FLAC__StreamDecoderTellStatus
iflac_tell(const FLAC__StreamDecoder *dec, FLAC__uint64 *pos,
	   void *client_data)
{
    struct igdata *g = client_data;
    long p;
    if (!g->seek)
	return (FLAC__STREAM_DECODER_TELL_STATUS_UNSUPPORTED);
    else if ((p = wfdb_ftell(g->fp)) < 0)
	return (FLAC__STREAM_DECODER_TELL_STATUS_ERROR);
    else {
	*pos = p;
	return (FLAC__STREAM_DECODER_TELL_STATUS_OK);
    }
}

/* iflac_length is called by the FLAC library in order to determine the
   total size of the input file. */
static FLAC__StreamDecoderLengthStatus
iflac_length(const FLAC__StreamDecoder *dec, FLAC__uint64 *len,
	     void *client_data)
{
    struct igdata *g = client_data;
    long pos, end;

    if (!g->seek)
	return (FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED);
    else if ((pos = wfdb_ftell(g->fp)) < 0)
	return (FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR);
    else if (wfdb_fseek(g->fp, 0L, SEEK_END) ||
	     (end = wfdb_ftell(g->fp)) < 0) {
	wfdb_fseek(g->fp, pos, SEEK_SET);
	return (FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR);
    }
    if (wfdb_fseek(g->fp, pos, SEEK_SET))
	return (FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR);
    *len = end;
    return (FLAC__STREAM_DECODER_LENGTH_STATUS_OK);
}

/* iflac_eof is called by the FLAC library in order to check whether
   we have reached the end of the input file. */
static FLAC__bool iflac_eof(const FLAC__StreamDecoder *dec, void *client_data)
{
    struct igdata *g = client_data;
    return (wfdb_feof(g->fp));
}

/* iflac_error is called by the FLAC library when the input stream
   appears invalid or corrupted. */
static void iflac_error(const FLAC__StreamDecoder *decoder,
			FLAC__StreamDecoderErrorStatus status,
			void *client_data)
{
    struct igdata *g = client_data;

    switch (status) {
      case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
	wfdb_error("getvec: unable to decode FLAC (lost sync)\n");
	break;
      case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
	wfdb_error("getvec: unable to decode FLAC (invalid header)\n");
	break;
      case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
	wfdb_error("getvec: unable to decode FLAC (CRC mismatch)\n");
	break;
      case FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM:
	wfdb_error("getvec: unable to decode FLAC (unsupported format)\n");
	break;
      default:
	wfdb_error("getvec: unable to decode FLAC\n");
	break;
    }
    /* Note that if an error is detected, the FLAC library will still
       subsequently invoke iflac_samples, with a buffer of zeroes
       rather than valid data. */
    g->stat = -2;
}

/* iflac_samples is called by the FLAC library when a block of samples
   has been decoded. */
static FLAC__StreamDecoderWriteStatus
iflac_samples(const FLAC__StreamDecoder *dec, const FLAC__Frame *ffrm,
	      const FLAC__int32 *const buf[], void *client_data)
{
    struct igdata *g = client_data;
    size_t nsig = ffrm->header.channels;
    size_t nsamp = ffrm->header.blocksize;
    size_t oldsize, newsize, frmsize, bufsize, spf, ipos, orem, s, n;
    char *nbuf;
    FLAC__int32 *p;

    /* g->data is the number of signals in the group. */
    if (nsig != g->data) {
	wfdb_error("getvec: wrong number of signals in FLAC signal file\n");
	g->stat = -2;
    }
    /* g->datb is the group sample resolution. */
    if (ffrm->header.bits_per_sample > g->datb) {
	wfdb_error("getvec: wrong sample resolution in FLAC signal file\n");
	g->stat = -2;
    }
    /* If the resolution or number of signals is incorrect, or if an
       error was previously detected by flac_error, then stop decoding
       immediately (and return an error from getskewedframe or
       isgsetframe.) */
    if (g->stat < 0)
	return (FLAC__STREAM_DECODER_WRITE_STATUS_ABORT);

    /* g->buf is the start of the input buffer.

       g->bp points to the next sample (FLAC__int32) to be retrieved.
       This advances by four bytes with each call to flac_getsamp.
       (When iflac_samples is called, g->bp should be located at the
       start of a frame.)

       g->packptr points to the next frame to be decoded.  This
       advances by 4*nsig*spf bytes when a complete frame has been
       decoded.

       g->packcount is the number of samples (of each signal) that
       have been decoded and stored at g->packptr.  This is initially
       zero; when it reaches spf, it is reset to zero and g->packptr
       advances.

       g->be is the end of the input buffer.  We should always have
       g->buf <= g->bp <= g->packptr <= g->be. */

    spf = g->packspf;
    newsize = nsig * nsamp * sizeof(FLAC__int32);
    frmsize = nsig * spf * sizeof(FLAC__int32);

    bufsize = g->be - g->buf;
    oldsize = g->packptr - g->bp;

    /* If the existing buffer is not large enough to hold old + new
       data, plus one extra frame, it must be reallocated.  Typically
       the FLAC block size is constant, so this reallocation will only
       be needed once per file. */
    if (bufsize < oldsize + newsize + frmsize) {
	bufsize = oldsize + newsize + frmsize;
	SUALLOC(nbuf, bufsize, 1);
	if (!nbuf)
	    return (FLAC__STREAM_DECODER_WRITE_STATUS_ABORT);

	n = oldsize + (g->packcount ? frmsize : 0);
	if (n)
	    memcpy(nbuf, g->bp, n);
	SFREE(g->buf);
	g->buf = g->bp = nbuf;
	g->be = nbuf + bufsize;
	g->packptr = nbuf + oldsize;
    }
    /* Otherwise, shift existing data (if any) to the beginning of the
       buffer. */
    else if (g->bp != g->buf) {
	n = oldsize + (g->packcount ? frmsize : 0);
	if (n)
	    memmove(g->buf, g->bp, n);
	g->bp = g->buf;
	g->packptr = g->buf + oldsize;
    }

    /* g->count is the number of initial samples to skip, following a
       seek (see flac_isseek). */
    if (g->count > nsamp) {
	g->count -= nsamp;
	return (FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE);
    }
    else {
	ipos = g->count;
	nsamp -= ipos;
	g->count = 0;
    }

    /* Rearrange data from the buffer(s) provided by the decoder, and
       store the samples in frame order. */
    p = ((FLAC__int32 *) g->packptr) + g->packcount;
    orem = spf - g->packcount;
    while (nsamp != 0) {
	n = (nsamp < orem ? nsamp : orem);
	for (s = 0; s < nsig; s++) {
	    memcpy(p, buf[s] + ipos, n * sizeof(FLAC__int32));
	    p += spf;
	}
	nsamp -= n;
	orem -= n;
	if (orem == 0) {
	    g->packptr += frmsize;
	    p = (FLAC__int32 *) g->packptr;
	    orem = spf;
	    ipos += n;
	}
    }
    g->packcount = spf - orem;

    return (FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE);
}

/* Read and return the next sample from a FLAC signal file. */
static int flac_getsamp(struct igdata *g)
{
    FLAC__int32 *ibp;
    FLAC__StreamDecoderState state;
    unsigned oldcount;

    /* If the next frame has not yet been decoded, read more data from
       the input file. */
    while (g->bp == g->packptr) {
	oldcount = g->packcount;
	if (!FLAC__stream_decoder_process_single(g->flacdec)) {
	    if (g->stat != -2) {
		wfdb_error("getvec: unexpected FLAC decoding error\n");
		g->stat = -2;
	    }
	    return (0);
	}
	if (g->stat <= 0) {
	    return (0);
	}
	if (g->bp == g->packptr && g->packcount == oldcount) {
	    state = FLAC__stream_decoder_get_state(g->flacdec);
	    switch (state) {
	      case FLAC__STREAM_DECODER_END_OF_STREAM:
		if (g->packcount != 0)
		    wfdb_error("getvec: warning: %d samples left over"
			       " at end of file\n", g->packcount);
		g->packcount = 0;
		g->stat = 0;
		return (0);
	      case FLAC__STREAM_DECODER_SEARCH_FOR_METADATA:
	      case FLAC__STREAM_DECODER_READ_METADATA:
	      case FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC:
		break;
	      default:
		wfdb_error("getvec: unknown FLAC decoding state\n");
		g->stat = -2;
		return (0);
	    }
	}
    }

    /* Retrieve the next decoded sample. */
    ibp = (FLAC__int32 *) g->bp;
    g->bp = (char *) &ibp[1];
    return (*ibp);
}

/* Open a FLAC stream decoder for an input signal group.  The input
   file (ig->fp) has already been opened and the buffer (ig->buf) has
   already been allocated. */
static int flac_isopen(struct igdata *ig, struct hsdata **hs, unsigned ns)
{
    unsigned int i;

    if (ns > FLAC__MAX_CHANNELS) {
	wfdb_error(
	    "isigopen: cannot store %u signals in a single FLAC file\n",
	    ns);
	return (-1);
    }
    for (i = 1; i < ns; i++) {
	if (hs[i]->info.spf != hs[0]->info.spf) {
	    wfdb_error(
	       "isigopen: every signal in a FLAC file must have\n"
	       "  the same sampling frequency\n");
	    return (-1);
	}
    }

    ig->flacdec = FLAC__stream_decoder_new();
    if (!ig->flacdec) {
	wfdb_error("isigopen: cannot initialize stream decoder\n");
	return (-1);
    }

    ig->data = ns;
    ig->datb = hs[0]->info.fmt - 500;
    ig->count = 0;
    ig->packspf = hs[0]->info.spf;
    ig->packptr = ig->be = ig->bp = ig->buf + ig->bsize;
    ig->packcount = 0;
    if (FLAC__stream_decoder_init_stream(ig->flacdec, &iflac_read,
					 &iflac_seek, &iflac_tell,
					 &iflac_length, &iflac_eof,
					 &iflac_samples, NULL,
					 &iflac_error, ig)) {
	wfdb_error("isigopen: cannot open stream decoder\n");
	FLAC__stream_decoder_delete(ig->flacdec);
	return (-1);
    }
    return (0);
}

/* Close a stream decoder. */
static int flac_isclose(struct igdata *ig)
{
    FLAC__stream_decoder_delete(ig->flacdec);
    return (0);
}

/* Seek to the given frame number in an input stream. */
static int flac_isseek(struct igdata *ig, WFDB_Time t)
{
    FLAC__StreamDecoderState state;
    WFDB_Time tt;

    /* If there was a previous seek error, clear the decoder state so
       we can try again.  (Calling flush at other times can break
       decoding for some reason.) */
    state = FLAC__stream_decoder_get_state(ig->flacdec);
    if (state == FLAC__STREAM_DECODER_SEEK_ERROR)
	FLAC__stream_decoder_flush(ig->flacdec);

    /* Reset the block pointers to indicate nothing has been read
       in the current block. */
    ig->packptr = ig->bp = ig->be;
    ig->packcount = 0;
    ig->stat = 1;
    ig->count = 0;

    /* Seek to the desired sample.  Note that seek_absolute will
       return an error if the given sample number is greater than or
       equal to the length of the stream; isgsetframe should succeed
       if tt is exactly the length of the stream.  Therefore, we
       subtract one from the target sample number, and set ig->count
       to 1 so that iflac_samples will discard the first sample from
       each signal. */
    tt = t * ig->packspf + ig->start;
    if (tt != 0) {
	tt--;
	ig->count = 1;
    }
    /* This will invoke iflac_samples, with a (partial) block of
       samples starting at the requested sample number. */
    if (!FLAC__stream_decoder_seek_absolute(ig->flacdec, tt))
	ig->stat = -1;
    return (ig->stat);
}

#else /* !WFDB_FLAC_SUPPORT */

static int flac_getsamp(struct igdata *ig)
{
    ig->stat = -1;
    return (0);
}

static int flac_isopen(struct igdata *ig, struct hsdata **hs, unsigned ns)
{
    wfdb_error("isigopen: libwfdb was compiled without FLAC support\n");
    return (-1);
}

static int flac_isclose(struct igdata *ig)
{
    return (-1);
}

static int flac_isseek(struct igdata *ig, WFDB_Time t)
{
    return (-1);
}

#endif

/* Routines for writing FLAC signal files. */

#ifdef WFDB_FLAC_SUPPORT

/* The following functions (oflac_write, oflac_seek, and oflac_tell)
   are used as callbacks for the FLAC library.  The client_data
   argument is the value passed to FLAC__stream_encoder_init_stream,
   which is a pointer to a struct ogdata. */

/* oflac_write is called by the FLAC library in order to write a
   chunk of data to the output file. */
static FLAC__StreamEncoderWriteStatus
oflac_write(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[],
	    size_t bytes, unsigned samples, unsigned current_frame,
	    void *client_data)
{
    struct ogdata *g = client_data;
    if (wfdb_fwrite(buffer, 1, bytes, g->fp) != bytes)
	return (FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR);
    else
	return (FLAC__STREAM_ENCODER_WRITE_STATUS_OK);
}

/* oflac_seek is called by the FLAC library in order to move to a
   different position in the output file. */
static FLAC__StreamEncoderSeekStatus
oflac_seek(const FLAC__StreamEncoder *encoder,
	   FLAC__uint64 pos, void *client_data)
{
    struct ogdata *g = client_data;
    if (pos > LONG_MAX)
	return (FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR);
    else if (wfdb_fseek(g->fp, pos, SEEK_SET))
	return (FLAC__STREAM_ENCODER_SEEK_STATUS_UNSUPPORTED);
    else
	return (FLAC__STREAM_ENCODER_SEEK_STATUS_OK);
}

/* oflac_tell is called by the FLAC library in order to determine
   the current position in the output file. */
static FLAC__StreamEncoderTellStatus
oflac_tell(const FLAC__StreamEncoder *encoder,
	   FLAC__uint64 *pos, void *client_data)
{
    struct ogdata *g = client_data;
    long p;
    if ((p = wfdb_ftell(g->fp)) < 0)
	return (FLAC__STREAM_ENCODER_TELL_STATUS_UNSUPPORTED);
    else {
	*pos = p;
	return (FLAC__STREAM_ENCODER_TELL_STATUS_OK);
    }
}

/* Write the next sample to a FLAC signal file. */
static int flac_putsamp(WFDB_Sample v, int fmt, struct ogdata *g)
{
    FLAC__int32 *obp;
    const FLAC__int32 *channels[FLAC__MAX_CHANNELS];
    unsigned spf, i;
    struct { WFDB_Sample v : 8; } v8;
    struct { WFDB_Sample v : 16; } v16;
    struct { WFDB_Sample v : 24; } v24;

    /* If sample values passed to the FLAC encoder exceed the
       specified number of "bits per sample", the result is
       unpredictable: the encoder might discard the upper bits, or it
       might actually encode some values outside the stated range.  To
       ensure that the results are consistent and predictable, even if
       the caller supplies invalid sample values to putvec, we discard
       the upper bits by sign-extending values before passing them to
       the encoder. */
    switch (fmt) {
      case 508: v = v8.v = v; break;
      case 516: v = v16.v = v; break;
      case 524: v = v24.v = v; break;
    }

    obp = (FLAC__int32 *) g->bp;
    *obp++ = v;

    /* The output buffer is exactly the size of one frame, so when obp
       reaches g->be we have one frame's worth of data to encode. */
    if (obp == (FLAC__int32 *) g->be) {
	obp = (FLAC__int32 *) g->buf;
	spf = g->packspf;
	for (i = 0; obp != (FLAC__int32 *) g->be; i++) {
	    channels[i] = obp;
	    obp += spf;
	}
	if (!FLAC__stream_encoder_process(g->flacenc, channels, spf)) {
	    wfdb_error("putvec: error writing FLAC signal data\n");
	    return (-1);
	}
	g->bp = g->buf;
	return (0);
    }
    else {
	g->bp = (char *) obp;
	return (0);
    }
}

static int flac_osinit(struct ogdata *og, const WFDB_Siginfo *si, unsigned ns)
{
    FLAC__StreamEncoder *enc;
    char *p;
    int min = 0, max = 0;
    unsigned int i;

    if (ns > FLAC__MAX_CHANNELS) {
	wfdb_error(
	    "osigfopen: cannot store %u signals in a single FLAC file\n",
	    ns);
	return (-1);
    }
    for (i = 1; i < ns; i++) {
	if (si[i].spf != si[0].spf) {
	    wfdb_error(
	       "osigfopen: every signal in a FLAC file must have\n"
	       "  the same sampling frequency\n");
	    return (-1);
	}
    }

    og->flacenc = enc = FLAC__stream_encoder_new();
    og->packspf = si->spf;
    if (!enc) {
	wfdb_error("osigfopen: cannot initialize stream encoder\n");
	return (-1);
    }
    FLAC__stream_encoder_set_channels(enc, ns);
    FLAC__stream_encoder_set_bits_per_sample(enc, si->fmt - 500);
    FLAC__stream_encoder_set_sample_rate(enc, 96000);

    /* If the output file is not seekable, set the STREAMINFO length
       to the maximum possible value.  If the length is zero, libFLAC
       will have problems when trying to read the file. */
    FLAC__stream_encoder_set_total_samples_estimate(enc, (FLAC__uint64) -1);

    /* Make the buffer exactly large enough to hold a single frame.
       (Also note that setting og->bsize to a non-zero value prevents
       wfdb_osflush from trying to flush the output buffer.) */
    og->bsize = ns * si->spf * sizeof(FLAC__int32);

    /* The following environment variables may be used to set
       parameters for the FLAC compression algorithm. */

    if (p = getenv("WFDB_FLAC_COMPRESSION_LEVEL"))
	FLAC__stream_encoder_set_compression_level(enc, atoi(p));
    else
	FLAC__stream_encoder_set_compression_level(enc, 5);

    if (p = getenv("WFDB_FLAC_BLOCK_SIZE"))
	FLAC__stream_encoder_set_blocksize(enc, atoi(p));
    if (p = getenv("WFDB_FLAC_STEREO")) {
	if (p[0] == 'a' || p[0] == 'A') { /* auto */
	    FLAC__stream_encoder_set_do_mid_side_stereo(enc, 1);
	    FLAC__stream_encoder_set_loose_mid_side_stereo(enc, 1);
	}
	else if (p[0] == 'b' || p[0] == 'B') { /* best */
	    FLAC__stream_encoder_set_do_mid_side_stereo(enc, 1);
	    FLAC__stream_encoder_set_loose_mid_side_stereo(enc, 0);
	}
	else {
	    FLAC__stream_encoder_set_do_mid_side_stereo(enc, 0);
	}
    }
    if (p = getenv("WFDB_FLAC_APODIZATION"))
	FLAC__stream_encoder_set_apodization(enc, p);
    if (p = getenv("WFDB_FLAC_MAX_LPC_ORDER"))
	FLAC__stream_encoder_set_max_lpc_order(enc, atoi(p));
    if (p = getenv("WFDB_FLAC_QLP_COEFF_PRECISION")) {
	if (p[0] == 'a' || p[0] == 'A') { /* auto */
	    FLAC__stream_encoder_set_qlp_coeff_precision(enc, 0);
	    FLAC__stream_encoder_set_do_qlp_coeff_prec_search(enc, 0);
	}
	else if (p[0] == 'b' || p[0] == 'B') { /* best */
	    FLAC__stream_encoder_set_qlp_coeff_precision(enc, 0);
	    FLAC__stream_encoder_set_do_qlp_coeff_prec_search(enc, 1);
	}
	else {
	    FLAC__stream_encoder_set_qlp_coeff_precision(enc, atoi(p));
	    FLAC__stream_encoder_set_do_qlp_coeff_prec_search(enc, 0);
	}
    }
    if (p = getenv("WFDB_FLAC_EXHAUSTIVE_MODEL_SEARCH")) {
	if (p[0] == 'y' || p[0] == 'Y')
	    FLAC__stream_encoder_set_do_exhaustive_model_search(enc, 1);
	else
	    FLAC__stream_encoder_set_do_exhaustive_model_search(enc, 0);
    }
    if (p = getenv("WFDB_FLAC_RICE_PARTITION_ORDER")) {
	min = strtol(p, &p, 10);
	if (p && *p == ',') {
	    max = strtol(p + 1, NULL, 10);
	}
	else {
	    max = min;
	    min = 0;
	}
	FLAC__stream_encoder_set_min_residual_partition_order(enc, min);
	FLAC__stream_encoder_set_max_residual_partition_order(enc, max);
    }

    return (0);
}

static int flac_osopen(struct ogdata *og)
{
    if (FLAC__stream_encoder_init_stream(og->flacenc, &oflac_write,
					 &oflac_seek, &oflac_tell,
					 NULL, og)) {
	wfdb_error("osigfopen: cannot open stream encoder\n");
	return (-1);
    }
    return (0);
}

static int flac_osclose(struct ogdata *og)
{
    int stat = 0;

    if (!FLAC__stream_encoder_finish(og->flacenc)) {
	wfdb_error("osigclose: error writing FLAC signal file\n");
	stat = -1;
    }
    FLAC__stream_encoder_delete(og->flacenc);
    og->bp = og->be = og->buf;
    return (stat);
}

#else /* !WFDB_FLAC_SUPPORT */

static int flac_putsamp(WFDB_Sample v, int fmt, struct ogdata *g)
{
    return (-1);
}

static int flac_osinit(struct ogdata *og, const WFDB_Siginfo *si, unsigned ns)
{
    wfdb_error("osigfopen: libwfdb was compiled without FLAC support\n");
    return (-1);
}

static int flac_osopen(struct ogdata *og)
{
    return (-1);
}

static int flac_osclose(struct ogdata *og)
{
    return (-1);
}

#endif

static void isigclose(void)
{
    struct isdata *is;
    struct igdata *ig;

    if (sbuf && !in_msrec) {
	SFREE(sbuf);
	sample_vflag = 0;
    }
    if (isd) {
	while (maxisig)
	    if (is = isd[--maxisig]) {
		SFREE(is->info.fname);
		SFREE(is->info.units);
		SFREE(is->info.desc);
		SFREE(is);
	    }
	SFREE(isd);
    }
    maxisig = nisig = 0;
    framelen = 0;

    if (igd) {
	while (maxigroup)
	    if (ig = igd[--maxigroup]) {
		if (ig->flacdec)
		    flac_isclose(ig);
		if (ig->fp) (void)wfdb_fclose(ig->fp);
		SFREE(ig->buf);
		SFREE(ig);
	    }
	SFREE(igd);
    }
    maxigroup = nigroup = 0;

    istime = 0L;
    gvc = ispfmax = 1;
    if (hheader) {
	(void)wfdb_fclose(hheader);
	hheader = NULL;
    }
    if (nosig == 0 && maxhsig != 0)
	hsdfree();
}

static int osigclose(void)
{
    struct osdata *os;
    struct ogdata *og;
    WFDB_Group g;
    int stat = 0, errflag;

    for (g = 0; g < nogroup; g++)
	if (ogd && (og = ogd[g]))
	    og->force_flush = 1;

    wfdb_osflush();

    if (osd) {
	while (maxosig)
	    if (os = osd[--maxosig]) {
		SFREE(os->info.fname);
		SFREE(os->info.units);
		SFREE(os->info.desc);
		SFREE(os);
	    }
	SFREE(osd);
    }
    nosig = 0;

    if (ogd) {
	while (maxogroup)
	    if (og = ogd[--maxogroup]) {
		if (og->fp) {
		    if (og->flacenc)
			flac_osclose(og);

		    /* If a block size has been defined, null-pad the buffer */
		    if (og->bsize)
			while (og->bp != og->be)
			    *(og->bp++) = '\0';
		    /* Flush the last block unless it's empty. */
		    if (og->bp != og->buf)
			(void)wfdb_fwrite(og->buf, 1, og->bp-og->buf, og->fp);
		    if (og->fp->fp == stdout)
			wfdb_fflush(og->fp);
		    errflag = wfdb_ferror(og->fp);
		    /* Close file (except stdout, which is closed on exit). */
		    if (og->fp->fp != stdout) {
			if (wfdb_fclose(og->fp))
			    errflag = 1;
			og->fp = NULL;
		    }
		    if (errflag) {
			wfdb_error("osigclose: write error"
				   " in signal group %d\n", maxogroup);
			stat = -4;
		    }
		}
		SFREE(og->buf);
		SFREE(og);
	    }
	SFREE(ogd);
    }
    maxogroup = nogroup = 0;

    ostime = 0L;
    if (oheader) {
	errflag = wfdb_ferror(oheader);
	if (wfdb_fclose(oheader))
	    errflag = 1;
	if (errflag) {
	    wfdb_error("osigclose: write error in header file\n");
	    stat = -4;
	}
	if (outinfo == oheader) outinfo = NULL;
	oheader = NULL;
    }
    if (nisig == 0 && maxhsig != 0)
	hsdfree();

    return (stat);
}

/* Low-level I/O routines.  The input routines each get a single argument (the
signal group pointer).  The output routines get two arguments (the value to be
written and the signal group pointer). */

static int _l;		    /* macro temporary storage for low byte of word */
static int _lw;		    /* macro temporary storage for low 16 bits of int */
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
#define r24(G)	    (_lw = r16(G), ((int)((r8(G) << 16) | (_lw & 0xffff))))
#define w24(V,G)    (w16((V), (G)), w8(((V) >> 16), (G)))
#define r32(G)	    (_lw = r16(G), ((int)((r16(G) << 16) | (_lw & 0xffff))))
#define w32(V,G)    (w16((V), (G)), w16(((V) >> 16), (G)))
#else

static int r16(struct igdata *g)
{
    int l, h;

    l = r8(g);
    h = r8(g);
    return ((int)((short)((h << 8) | (l & 0xff))));
}

static void w16(WFDB_Sample v, struct ogdata *g)
{
    w8(v, g);
    w8((v >> 8), g);
}

static int r61(struct igdata *g)
{
    int l, h;

    h = r8(g);
    l = r8(g);
    return ((int)((short)((h << 8) | (l & 0xff))));
}

static void w61(WFDB_Sample v, struct ogdata *g)
{
    w8((v >> 8), g);
    w8(v, g);
}

/* r24: read and return the next sample from a format 24 signal file */
static int r24(struct igdata *g)
{
    int l, h;

    l = r16(g);
    h = r8(g);
    return ((int)((h << 16) | (l & 0xffff)));
}

/* w24: write the next sample to a format 24 signal file */
static void w24(WFDB_Sample v, struct ogdata *g)
{
    w16(v, g);
    w8((v >> 16), g);
}

/* r32: read and return the next sample from a format 32 signal file */
static int r32(struct igdata *g)
{
    int l, h;

    l = r16(g);
    h = r16(g);
    return ((int)((h << 16) | (l & 0xffff)));
}

/* w32: write the next sample to a format 32 signal file */
static void w32(WFDB_Sample v, struct ogdata *g)
{
    w16(v, g);
    w16((v >> 16), g);
}
#endif

#define r80(G)		((r8(G) & 0xff) - (1 << 7))
#define w80(V, G)	(w8(((V) & 0xff) + (1 << 7), G))

#define r160(G)		((r16(G) & 0xffff) - (1 << 15))
#define w160(V, G)	(w16(((V) & 0xffff) + (1 << 15), G))

/* r212: read and return the next sample from a format 212 signal file
   (2 12-bit samples bit-packed in 3 bytes) */
static int r212(struct igdata *g)
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

/* w212: write the next sample to a format 212 signal file */
static void w212(WFDB_Sample v, struct ogdata *g)                 
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

/* f212: flush output to a format 212 signal file */
static void f212(struct ogdata *g)
{
    /* If we have one leftover sample, write it as two bytes. */
    if (g->count == 1) {
	w16(g->data, g);
	g->nrewind = 2;
    }
}

/* r310: read and return the next sample from a format 310 signal file
   (3 10-bit samples bit-packed in 4 bytes) */
static int r310(struct igdata *g)
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

/* w310: write the next sample to a format 310 signal file */
static void w310(WFDB_Sample v, struct ogdata *g)                 
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

/* f310: flush output to a format 310 signal file */
static void f310(struct ogdata *g)
{
    switch (g->count) {
      case 0:  break;
      /* If we have one leftover sample, write it as two bytes. */
      case 1:  w16(g->data, g);
	       g->nrewind = 2;
	       break;
      /* If we have two leftover samples, write them as four bytes.
	 In this case, the file will appear to have an extra (zero)
	 sample at the end. */
      case 2:
      default: w16(g->data, g);
	       w16(g->datb, g);
	       g->nrewind = 4;
	       break;
    }
}

/* r311: read and return the next sample from a format 311 signal file
   (3 10-bit samples bit-packed in 4 bytes; note that formats 310 and 311
   differ in the layout of the bit-packed data) */
static int r311(struct igdata *g)
{
    int v;

    /* Obtain the next 10-bit value right-justified in v. */
    switch (g->count++) {
      case 0:	v = (g->data = r16(g)); break;
      case 1:	g->datb = (r8(g) & 0xff);
	        v = ((g->data & 0xfc00) >> 10) | ((g->datb & 0xf) << 6);
		break;
      case 2:
      default:	g->count = 0;
		g->datb |= r8(g) << 8;
		v = g->datb >> 4; break;
    }
    /* Sign-extend from the tenth bit. */
    if (v & 0x200) v |= ~(0x3ff);
    else v &= 0x3ff;
    return (v);
}

/* w311: write the next sample to a format 311 signal file */
static void w311(WFDB_Sample v, struct ogdata *g)                 
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

/* f311: flush output to a format 311 signal file */
static void f311(struct ogdata *g)
{
    switch (g->count) {
      case 0:	break;
      /* If we have one leftover sample, write it as two bytes. */
      case 1:	w16(g->data, g);
		g->nrewind = 2;
		break;
      /* If we have two leftover samples, write them as four bytes
	 (note that the first two bytes will already have been written
	 by w311(), above.)  The file will appear to have an extra
	 (zero) sample at the end.  It would be possible to write only
	 three bytes here, but older versions of WFDB would not be
	 able to read the resulting file. */
      case 2:
      default:	w16(g->datb, g);
		g->nrewind = 2;
		break;
    }
}

static int isgsetframe(WFDB_Group g, WFDB_Time t)
{
    int i, trem = 0;
    long nb, tt;
    struct igdata *ig;
    WFDB_Signal s;
    unsigned int b, d = 1, n, nn, j;

    /* Do nothing if there is no more than one input signal group and
       the input pointer is correct already. */
    if (nigroup < 2 && istime == t && gvc == ispfmax &&
	igd[g]->start == 0)
	return (0);

    /* Find the first signal that belongs to group g. */
    for (s = 0; s < nisig && g != isd[s]->info.group; s++)
	;
    if (s == nisig) {
	wfdb_error("isgsettime: incorrect signal group number %d\n", g);
	return (-2);
    }

    /* If the current record contains multiple segments, locate the segment
       containing the desired sample. */
    if (in_msrec) {
	WFDB_Seginfo *tseg = segp;
	WFDB_Group h;

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
	    if (isigopen(segp->recname, NULL, (int)nvsig) <= 0) {
	        wfdb_error("isigsettime: can't open segment %s\n",
			   segp->recname);
		return (-1);
	    }
	    /* Following isigopen(), nigroup may have changed and
	       group numbers may not make any sense anymore.  However,
	       isigsettime() will still call isgsettime() once for
	       each non-zero group (if the previous segment had
	       multiple groups) and then once for group zero.

	       Calling isgsetframe() multiple times for a non-zero
	       group is mostly harmless, but seeking on group 0 should
	       only be done once.  Thus, when we jump to a new
	       segment, implicitly seek on all non-zero groups
	       (regardless of g), but only seek on group 0 if g is 0.

	       (Note that isgsettime() is not and has never been fully
	       functional for multi-segment records, because it cannot
	       read signals from two different segments at once.) */
	    for (h = 1; h < nigroup; h++)
		if (i = isgsetframe(h, t))
		    return (i);
	    if (g == 0)
		return (isgsetframe(0, t));
	    else
		return (0);
	}
	t -= segp->samp0;
    }

    ig = igd[g];
    ig->initial_skip = 0;
    /* Determine the number of samples per frame for signals in the group. */
    for (n = nn = 0; s+n < nisig && isd[s+n]->info.group == g; n++)
	nn += isd[s+n]->info.spf;
    /* Determine the number of bytes per sample interval in the file. */
    switch (isd[s]->info.fmt) {
      case 0:
	if (t < nsamples) {
	    gvc = ispfmax;
	    if (s == 0) istime = (in_msrec) ? t + segp->samp0 : t;
	    isd[s]->info.nsamp = nsamples - t;
	    ig->stat = 1;
	    return (0);
	}
	else {
	    if (s == 0) istime = (in_msrec) ? msnsamples : nsamples;
	    isd[s]->info.nsamp = 0L;
	    return (-1);
	}

      case 508:
      case 516:
      case 524:
	if (flac_isseek(ig, t) < 0) {
	    wfdb_error("isigsettime: improper seek on signal group %d\n", g);
	    return (-1);
	}

	/* Reset the getvec sample-within-frame counter. */
	gvc = ispfmax;

	/* Reset the time (if signal 0 belongs to the group) and
	   disable checksum testing (by setting the number of samples
	   remaining to 0). */
	if (s == 0) istime = in_msrec ? t + segp->samp0 : t;
	while (n-- != 0)
	    isd[s+n]->info.nsamp = (WFDB_Time)0L;
	return (0);

      case 8:
      case 80:
      default: b = nn; break;
      case 16:
      case 61:
      case 160:
	if (nn > UINT_MAX / 2) {
	    wfdb_error("isigsettime: overflow in signal group %d\n", g);
	    return (-1);
	}
	b = 2*nn;
	break;
      case 212:
	if (nn > UINT_MAX / 3) {
	    wfdb_error("isigsettime: overflow in signal group %d\n", g);
	    return (-1);
	}
	/* Reset the input counter. */
	ig->count = 0;
	/* If the desired sample does not lie on a byte boundary, seek to
	   the previous sample and then read ahead. */
	if ((nn & 1) && (t & 1)) {
	    if (in_msrec)
		t += segp->samp0;	/* restore absolute time */
	    if (i = isgsetframe(g, t - 1))
		return (i);
	    for (j = 0; j < nn; j++)
		(void)r212(ig);
	    istime++;
	    for (n = 0; s+n < nisig && isd[s+n]->info.group == g; n++)
		isd[s+n]->info.nsamp = (WFDB_Time)0L;
	    return (0);
	}
	b = 3*nn; d = 2; break;
      case 310:
	if (nn > UINT_MAX / 4) {
	    wfdb_error("isigsettime: overflow in signal group %d\n", g);
	    return (-1);
	}
	/* Reset the input counter. */
	ig->count = 0;
	/* If the desired sample does not lie on a byte boundary, seek to
	   the closest previous sample that does, then read ahead. */
	if ((nn % 3) && (trem = (t % 3))) {
	    if (in_msrec)
		t += segp->samp0;	/* restore absolute time */
	    if (i = isgsetframe(g, t - trem))
		return (i);
	    for (j = nn*trem; j > 0; j--)
		(void)r310(ig);
	    istime += trem;
	    for (n = 0; s+n < nisig && isd[s+n]->info.group == g; n++)
		isd[s+n]->info.nsamp = (WFDB_Time)0L;
	    return (0);
	}		  
	b = 4*nn; d = 3; break;
      case 311:
	if (nn > UINT_MAX / 4) {
	    wfdb_error("isigsettime: overflow in signal group %d\n", g);
	    return (-1);
	}
	/* Reset the input counter. */
	ig->count = 0;
	/* If the desired sample does not lie on a byte boundary, seek to
	   the closest previous sample that does, then read ahead. */
	if ((nn % 3) && (trem = (t % 3))) {
	    if (in_msrec)
		t += segp->samp0;	/* restore absolute time */
	    if (i = isgsetframe(g, t - trem))
		return (i);
	    for (j = nn*trem; j > 0; j--)
		(void)r311(ig);
	    istime += trem;
	    for (n = 0; s+n < nisig && isd[s+n]->info.group == g; n++)
		isd[s+n]->info.nsamp = (WFDB_Time)0L;
	    return (0);
	}		  
	b = 4*nn; d = 3; break;
      case 24:
	if (nn > UINT_MAX / 3) {
	    wfdb_error("isigsettime: overflow in signal group %d\n", g);
	    return (-1);
	}
	b = 3*nn;
	break;
      case 32:
	if (nn > UINT_MAX / 4) {
	    wfdb_error("isigsettime: overflow in signal group %d\n", g);
	    return (-1);
	}
	b = 4*nn;
	break;
    }

    if (t > (LONG_MAX / b) || ((long) (t * b))/d > (LONG_MAX - ig->start)) {
	wfdb_error("isigsettime: improper seek on signal group %d\n", g);
	return (-1);
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
	WFDB_Time t0, t1;

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

/* VFILL provides the value returned by getskewedframe() for a missing or
   invalid sample */
#define VFILL	((gvmode & WFDB_GVPAD) ? is->samp : WFDB_INVALID_SAMPLE)

static int getskewedframe(WFDB_Sample *vector)
{
    int c, stat;
    struct isdata *is;
    struct igdata *ig;
    WFDB_Group g;
    WFDB_Sample v, *vecstart = vector;
    WFDB_Signal s;

    if ((stat = (int)nisig) == 0) return (nvsig > 0 ? -1 : 0);
    if (istime == 0L) {
	for (s = 0; s < nisig; s++)
	    isd[s]->samp = isd[s]->info.initval;
    }
    for (g = nigroup; g; ) {
	/* Go through groups in reverse order since seeking on group 0
	   should always be done last. */
	if (igd[--g]->initial_skip)
	    isgsetframe(g, (in_msrec ? segp->samp0 : 0));
    }

    /* If the vector needs to be rearranged (variable-layout record),
       then read samples into a temporary buffer. */
    if (need_sigmap)
	vector = ovec;

    for (s = 0; s < nisig; s++) {
	is = isd[s];
	ig = igd[is->info.group];
	for (c = 0; c < is->info.spf; c++, vector++) {
	    switch (is->info.fmt) {
	      case 0:	/* null signal: return sample tagged as invalid */
		  *vector = v = VFILL;
		if (is->info.nsamp == 0) ig->stat = -1;
		break;
	      case 8:	/* 8-bit first differences */
	      default:
		*vector = v = is->samp += r8(ig); break;
	      case 16:	/* 16-bit amplitudes */
		*vector = v = r16(ig);
		if (v == -1 << 15)
		    *vector = VFILL;
		else
		    is->samp = *vector;
		break;
	      case 61:	/* 16-bit amplitudes, bytes swapped */
		*vector = v = r61(ig);
		if (v == -1 << 15)
		    *vector = VFILL;
		else
		    is->samp = *vector;
		break;
	      case 80:	/* 8-bit offset binary amplitudes */
		*vector = v = r80(ig);
		if (v == -1 << 7)
		    *vector = VFILL;
		else
		    is->samp = *vector;
		break;
	      case 160:	/* 16-bit offset binary amplitudes */
		*vector = v = r160(ig);
		if (v == -1 << 15)
		    *vector = VFILL;
		else
		    is->samp = *vector;
		break;
	      case 212:	/* 2 12-bit amplitudes bit-packed in 3 bytes */
		*vector = v = r212(ig);
		if (v == -1 << 11)
		    *vector = VFILL;
		else
		    is->samp = *vector;
		break;
	      case 310:	/* 3 10-bit amplitudes bit-packed in 4 bytes */
		*vector = v = r310(ig);
		if (v == -1 << 9)
		    *vector = VFILL;
		else
		    is->samp = *vector;
		break;
	      case 311:	/* 3 10-bit amplitudes bit-packed in 4 bytes */
		*vector = v = r311(ig);
		if (v == -1 << 9)
		    *vector = VFILL;
		else
		    is->samp = *vector;
		break;
	      case 24:	/* 24-bit amplitudes */
		*vector = v = r24(ig);
		if (v == -1 << 23)
		    *vector = VFILL;
		else
		    is->samp = *vector;
		break;
	      case 32:	/* 32-bit amplitudes */
		*vector = v = r32(ig);
		if (v == -1 << 31)
		    *vector = VFILL;
		else
		    is->samp = *vector;
		break;
	      case 508:	/* 8-bit compressed FLAC */
		*vector = v = flac_getsamp(ig);
		if (v == -1 << 7)
		    *vector = VFILL;
		else
		    is->samp = *vector;
		break;
	      case 516:	/* 16-bit compressed FLAC */
		*vector = v = flac_getsamp(ig);
		if (v == -1 << 15)
		    *vector = VFILL;
		else
		    is->samp = *vector;
		break;
	      case 524:	/* 24-bit compressed FLAC */
		*vector = v = flac_getsamp(ig);
		if (v == -1 << 23)
		    *vector = VFILL;
		else
		    is->samp = *vector;
		break;
	    }
	    if (ig->stat <= 0) {
		/* End of file -- reset input counter. */
		ig->count = 0;
		if (ig->stat == -2) {
		    /* error in decoding compressed data */
		    stat = -3;
		}
		else if (is->info.nsamp > (WFDB_Time)0L) {
		    wfdb_error("getvec: unexpected EOF in signal %d\n", s);
		    stat = -3;
		}
		else if (in_msrec && segp && segp < segend) {
		    segp++;
		    if (isigopen(segp->recname, NULL, (int)nvsig) <= 0) {
			wfdb_error("getvec: error opening segment %s\n",
				   segp->recname);
			stat = -3;
			return (stat);  /* avoid looping if segment is bad */
		    }
		    else {
			istime = segp->samp0;
			return (getskewedframe(vecstart));
		    }
		}
		else
		    stat = -1;
	    }
	    is->info.cksum -= v;
	}
	if (is->info.nsamp >= 0 && --is->info.nsamp == 0 &&
	    (is->info.cksum & 0xffff) &&
	    !in_msrec && !isedf &&
	    is->info.fmt != 0) {
	    wfdb_error("getvec: checksum error in signal %d\n", s);
	    stat = -4;
	}
    }

    if (need_sigmap)
	sigmap(vecstart, ovec);
    else if (framelen != tspf)
	for (s = framelen; s < tspf; s++)
	    vecstart[s] = WFDB_INVALID_SAMPLE;

    return (stat);
}

/* meansamp: calculate the mean of n sample values.  The result is
   rounded to the nearest integer, with halfway cases always rounded
   up. */
static WFDB_Sample meansamp(const WFDB_Sample *s, int n)
{
    /* If a WFDB_Time is large enough to hold the sum of the sample
       values, then simply add them up and divide by n. */
    if (WFDB_SAMPLE_MAX <= WFDB_TIME_MAX / INT_MAX) {
	WFDB_Time sum = n / 2;
	int i;

	for (i = 0; i < n; i++) {
	    if (*s == WFDB_INVALID_SAMPLE)
		return (WFDB_INVALID_SAMPLE);
	    sum += *s++;
	}
	if (n & (n - 1)) {	/* if n is not a power of two */
	    return (sum + (sum < 0)) / n - (sum < 0);
	}
	else {
	    while ((n /= 2) != 0)
		sum = (sum - (sum < 0)) / 2;
	    return sum;
	}
    }
    /* Otherwise, calculate the quotient and remainder separately to
       avoid overflows. */
    else {
	WFDB_Sample q, qq = 0;
	int i, r, rr = n / 2;

	for (i = 0; i < n; i++) {
	    if (*s == WFDB_INVALID_SAMPLE)
		return (WFDB_INVALID_SAMPLE);
	    q = *s / n;
	    r = *s % n;
	    if (r >= 0) {
		q++;
		r -= n;
	    }
	    qq += q;
	    rr += r;
	    if (rr < 0) {
		rr += n;
		qq--;
	    }
	    s++;
	}
	return (qq);
    }
}

static int rgetvec(WFDB_Sample *vector)
{
    WFDB_Sample *tp;
    WFDB_Signal s;
    static int stat;

    if (ispfmax < 2)	/* all signals at the same frequency */
	return (getframe(vector));

    if ((gvmode & WFDB_HIGHRES) != WFDB_HIGHRES) {
	/* return one sample per frame, decimating by averaging if necessary */
	stat = getframe(tvector);
	for (s = 0, tp = tvector; s < nvsig; s++) {
	    int sf = vsd[s]->info.spf;
	    *vector++ = meansamp(tp, sf);
	    tp += sf;
	}
    }
    else {			/* return ispfmax samples per frame, using
				   zero-order interpolation if necessary */
	if (gvc >= ispfmax) {
	    stat = getframe(tvector);
	    gvc = 0;
	}
	for (s = 0, tp = tvector; s < nvsig; s++) {
	    int sf = vsd[s]->info.spf;
	    if (gvc == 0) {
		vsd[s]->gvindex = 0;
		vsd[s]->gvcount = -ispfmax;
	    }
	    else {
		vsd[s]->gvcount += sf;
		if (vsd[s]->gvcount >= 0) {
		    vsd[s]->gvindex++;
		    vsd[s]->gvcount -= ispfmax;
		}
	    }
	    *vector++ = tp[vsd[s]->gvindex];
	    tp += sf;
	}
	gvc++;
    }
    return (stat);
}

/* WFDB library functions. */

FINT isigopen(char *record, WFDB_Siginfo *siarray, int nsig)
{
    int navail, nn, spflimit;
    int first_segment = 0;
    struct hsdata *hs;
    struct isdata *is;
    struct igdata *ig;
    WFDB_Signal s, si, sj;
    WFDB_Group g;

    /* Close previously opened input signals unless otherwise requested. */
    if (*record == '+') record++;
    else isigclose();

    /* Remove trailing .hea, if any, from record name. */
    wfdb_striphea(record);

    /* Save the current record name. */
    if (!in_msrec) wfdb_setirec(record);

    /* Read the header and determine how many signals are available. */
    if ((navail = readheader(record)) <= 0) {
	if (navail == 0 && segments) {	/* this is a multi-segment record */
	    in_msrec = 1;
	    first_segment = 1;
	    /* Open the first segment to get signal information. */
	    if (segp && (navail = readheader(segp->recname)) >= 0) {
		if (msbtime == 0L) msbtime = btime;
		if (msbdate == (WFDB_Date)0) msbdate = bdate;
	    }
	    if (nsig <= 0)
		in_msrec = 0;
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
    nn = nigroup + hsd[navail-1]->info.group + 1;
    if (nn > nigroup + nsig)
	nn = nigroup + nsig;
    if (allocigroup(nn) != nn)
	return (-1);	/* failed, allocigroup emits error */

    /* Set default buffer size (if not set already by setibsize). */
    if (ibsize <= 0) ibsize = BUFSIZ;
  
    /* Open the signal files.  One signal group is handled per iteration.  In
       this loop, si counts through the entries that have been read from hsd,
       and s counts the entries that have been added to isd. */
    for (g = si = s = 0; si < navail && s < nsig; si = sj) {
        hs = hsd[si];
	is = isd[nisig+s];
	ig = igd[nigroup+g];

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
	SALLOC(ig->buf, 1, ig->bsize);

	/* Check that the signal file is readable. */
	if (hs->info.fmt == 0)
	    ig->fp = NULL;	/* Don't open a file for a null signal. */
	else { 
	    ig->fp = wfdb_open(hs->info.fname, (char *)NULL, WFDB_READ);
	    /* Skip this group if the signal file can't be opened. */
	    if (ig->fp == NULL) {
	        SFREE(ig->buf);
		continue;
	    }
	}

	if (isflacfmt(hs->info.fmt)) {
	    if (flac_isopen(ig, &hsd[si], sj - si) < 0) {
		SFREE(ig->buf);
		wfdb_fclose(ig->fp);
		continue;
	    }
	}

	/* All tests passed -- fill in remaining data for this group. */
	ig->be = ig->bp = ig->buf + ig->bsize;
	ig->start = hs->start;
	ig->initial_skip = (ig->start > 0);
	ig->stat = 1;
	while (si < sj && s < nsig) {
	    copysi(&is->info, &hs->info);
	    is->info.group = nigroup + g;
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

    /* Check that the total number of samples per frame is less than
       or equal to INT_MAX. */
    spflimit = INT_MAX - framelen;
    for (si = 0; si < s; si++) {
	spflimit -= isd[nisig + si]->info.spf;
	if (spflimit < 0) {
	    wfdb_error("isigopen: total frame size too large in record %s\n",
		       record);
	    isigclose();
	    return (-3);
	}
    }

    /* Copy the WFDB_Siginfo structures to the caller's array.  Use these
       data to construct the initial sample vector, and to determine the
       maximum number of samples per signal per frame and the maximum skew. */
    for (si = 0; si < s; si++) {
        is = isd[nisig + si];
	if (siarray) 
	    copysi(&siarray[si], &is->info);
	is->samp = is->info.initval;
	if (ispfmax < is->info.spf) ispfmax = is->info.spf;
	if (skewmax < is->skew) skewmax = is->skew;
    }
    nisig += s;		/* Update the count of open input signals. */
    nigroup += g;	/* Update the count of open input signal groups. */
    if (sigmap_init(first_segment) < 0) {
	isigclose();
	return (-3);
    }
    spfmax = ispfmax;
    setgvmode(gvmode);	/* Reset sfreq if appropriate. */
    gvc = ispfmax;	/* Initialize getvec's sample-within-frame counter. */

    /* Determine the total number of samples per frame. */
    for (si = framelen = 0; si < nisig; si++)
	framelen += isd[si]->info.spf;

    /* Allocate workspace for getvec, isgsettime, and tnextvec. */
    if (tspf > tuvlen) {
	SALLOC(tvector, tspf, sizeof(WFDB_Sample));
	SALLOC(uvector, tspf, sizeof(WFDB_Sample));
	SALLOC(vvector, tspf, sizeof(WFDB_Sample));
	tuvlen = tspf;
    }

    /* If deskewing is required, allocate the deskewing buffer (unless this is
       a multi-segment record and dsbuf has been allocated already). */
    if (skewmax != 0 && (!in_msrec || dsbuf == NULL)) {
	if (tspf > INT_MAX / (skewmax + 1)) {
	    wfdb_error("isigopen: maximum skew too large in record %s\n",
		       record);
	    isigclose();
	    return (-3);
	}
	dsbi = -1;	/* mark buffer contents as invalid */
	dsblen = tspf * (skewmax + 1);
	SALLOC(dsbuf, dsblen, sizeof(WFDB_Sample));
    }
    return (s);
}

static int openosig(const char *func, WFDB_Siginfo *si_out,
		    const WFDB_Siginfo *si_in, unsigned int nsig)
{
    struct osdata *os, *op;
    struct ogdata *og;
    WFDB_Signal s;
    unsigned int ga;

    /* Allocate workspace for output signals. */
    if (allocosig(nosig + nsig) < 0) return (-3);
    /* Allocate workspace for output signal groups. */
    if (allocogroup(nogroup + si_in[nsig-1].group + 1) < 0) return (-3);

    /* Initialize local variables. */
    if (obsize <= 0) obsize = BUFSIZ;

    /* Set the group number adjustment.  This quantity is added to the group
       numbers of signals which are opened below;  it accounts for any output
       signals which were left open from previous calls. */
    ga = nogroup;

    /* Open the signal files.  One signal is handled per iteration. */
    for (s = 0, os = osd[nosig]; s < nsig; s++, nosig++, si_in++) {
	op = os;
	os = osd[nosig];

	/* Copy signal information from the caller's array. */
	copysi(&os->info, si_in);
	if (os->info.spf < 1) os->info.spf = 1;
	os->info.cksum = 0;
	os->info.nsamp = (WFDB_Time)0L;
	os->info.group += ga;
	if (si_out) {
	    copysi(si_out, &os->info);
	    si_out++;
	}

	/* Check if this signal is in the same group as the previous one. */
	if (s == 0 || os->info.group != op->info.group) {
	    size_t obuflen;

	    og = ogd[os->info.group];
	    og->bsize = os->info.bsize;
	    if (isflacfmt(os->info.fmt)) {
		unsigned ns = 1;
		while (s + ns < nsig && si_in[ns].group == si_in[0].group)
		    ns++;
		if (flac_osinit(og, si_in, ns) < 0) {
		    osigclose();
		    return (-3);
		}
	    }

	    obuflen = og->bsize ? og->bsize : obsize;
	    /* This is the first signal in a new group; allocate buffer. */
	    SALLOC(og->buf, 1, obuflen);
	    og->bp = og->buf;
	    og->be = og->buf + obuflen;
	    if (os->info.fmt == 0) {
		/* If the signal file name was NULL or "~", don't create a
		   signal file. */
		if (os->info.fname == NULL || strcmp("~", os->info.fname) == 0)
		    og->fp = NULL;
		/* Otherwise, assume that the user wants to write a signal
		   file in the default format (16). */
		else
		    os->info.fmt = 16;
	    }
	    if (os->info.fmt != 0) {
		/* An error in opening an output file is fatal. */
		og->fp = wfdb_open(os->info.fname,(char *)NULL, WFDB_WRITE);
		if (og->fp == NULL) {
		    wfdb_error("%s: can't open %s\n", func, os->info.fname);
		    SFREE(og->buf);
		    osigclose();
		    return (-3);
		}
	    }
	    if (isflacfmt(os->info.fmt) && flac_osopen(og) < 0) {
		SFREE(og->buf);
		osigclose();
		return (-3);
	    }
	    nogroup++;
	}
	else {
	    /* This signal belongs to the same group as the previous signal. */
	    if (os->info.fmt != op->info.fmt ||
		os->info.bsize != op->info.bsize) {
		wfdb_error(
		    "%s: error in specification of signal %d or %d\n",
		    func, s-1, s);
		return (-2);
	    }
	}
    }
    return (s);
}

FINT osigopen(char *record, WFDB_Siginfo *siarray, unsigned int nsig)
{
    int n;
    WFDB_Signal s;
    WFDB_Siginfo *hsi;

    /* Close previously opened output signals unless otherwise requested. */
    if (*record == '+') record++;
    else osigclose();

    /* Remove trailing .hea, if any, from record name. */
    wfdb_striphea(record);

    if ((n = readheader(record)) < 0)
	return (n);
    if (n < nsig) {
	wfdb_error("osigopen: record %s has fewer signals than needed\n",
		 record);
	return (-3);
    }

    SUALLOC(hsi, nsig, sizeof(WFDB_Siginfo));
    if (!hsi) return (-3);
    for (s = 0; s < nsig; s++)
	hsi[s] = hsd[s]->info;
    s = openosig("osigopen", siarray, hsi, nsig);
    SFREE(hsi);
    return (s);
}

FINT osigfopen(const WFDB_Siginfo *siarray, unsigned int nsig)
{
    int s, stat;
    const WFDB_Siginfo *si;

    /* Close any open output signals. */
    stat = osigclose();

    /* Do nothing further if there are no signals to open. */
    if (siarray == NULL || nsig == 0) return (stat);

    if (obsize <= 0) obsize = BUFSIZ;

    /* Prescan siarray to check the signal specifications and to determine
       the number of signal groups. */
    for (s = 0, si = siarray; s < nsig; s++, si++) {
	/* The combined lengths of the fname and desc strings should be 200
	   characters or less, the bsize field must not be negative, the
	   format should be legal, group numbers should be the same if and
	   only if file names are the same, and group numbers should begin
	   at zero and increase in steps of 1. */
	if (strlen(si->fname) + strlen(si->desc) > 200 ||
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

    return (openosig("osigfopen", NULL, siarray, nsig));
}

/* Function findsig finds an open input signal with the name specified by its
(string) argument, and returns the associated signal number.  If the argument
is a decimal numeral and is less than the number of open input signals, it is
assumed to represent a signal number, which is returned.  Otherwise, findsig
looks for a signal with a description matching the string, and returns the
first match if any, or -1 if not. */

int findsig(const char *p)
{
  const char *q = p;
  int s;

  while ('0' <= *q && *q <= '9')
      q++;
  if (*q == 0) {	/* all digits, probably a signal number */
      s = strtol(p, NULL, 10);
      if (s < nisig || s < nvsig) return (s);
  }
  /* Otherwise, p is either an integer too large to be a signal number or a
     string containing a non-digit character.  Assume it's a signal name. */
  if (need_sigmap) {
      for (s = 0; s < nvsig; s++)
	  if ((q = vsd[s]->info.desc) && strcmp(p, q) == 0) return (s);
  }
  else {
      for (s = 0; s < nisig; s++)
	  if ((q = isd[s]->info.desc) && strcmp(p, q) == 0) return (s);
  }
  /* No match found. */
  return (-1);
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

FINT getspf(void)
{
    return ((sfreq != ffreq) ? (int)(sfreq/ffreq + 0.5) : 1);
}

FVOID setgvmode(int mode)
{
    if (mode < 0) {	/* (re)set to default mode */
	char *p;

        if (p = getenv("WFDBGVMODE"))
	    mode = strtol(p, NULL, 10);
	else
	    mode = DEFWFDBGVMODE;
    }

    gvmode = mode & (WFDB_HIGHRES | WFDB_GVPAD);

    if ((mode & WFDB_HIGHRES) == WFDB_HIGHRES) {
	if (spfmax == 0) spfmax = 1;
	sfreq = ffreq * spfmax;
    }
    else {
	sfreq = ffreq;
    }
}

FINT getgvmode(void)
{
    return (gvmode);
}

/* An application can specify the input sampling frequency it prefers by
   calling setifreq after opening the input record. */

static long mticks, nticks, mnticks;
static int rgvstat;
static WFDB_Time rgvtime, gvtime;
static WFDB_Sample *gv0, *gv1;

FINT setifreq(WFDB_Frequency f)
{
    WFDB_Frequency error, g = sfreq;

    if (g <= 0.0) {
	ifreq = 0.0;
	wfdb_error("setifreq: no open input record\n");
	return (-1);
    }
    if (f > 0.0) {
	if (nvsig > 0) {
	    SREALLOC(gv0, nvsig, sizeof(WFDB_Sample));
	    SREALLOC(gv1, nvsig, sizeof(WFDB_Sample));
	}
	setafreq(ifreq = f);
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

FINT getvec(WFDB_Sample *vector)
{
    int i, nsig;

    if (ifreq == 0.0 || ifreq == sfreq)	/* no resampling necessary */
	return (rgetvec(vector));

    /* Resample the input. */
    if (rgvtime > mnticks) {
	rgvtime -= mnticks;
	gvtime  -= mnticks;
    }
    nsig = (nvsig > nisig) ? nvsig : nisig;
    while (gvtime > rgvtime) {
	for (i = 0; i < nsig; i++)
	    gv0[i] = gv1[i];
	rgvstat = rgetvec(gv1);
	rgvtime += nticks;
    }
    for (i = 0; i < nsig; i++) {
	vector[i] = gv0[i] + (gvtime%nticks)*(gv1[i]-gv0[i])/nticks;
        gv0[i] = gv1[i];
    }
    gvtime += mticks;
    return (rgvstat);
}

FINT getframe(WFDB_Sample *vector)
{
    int stat = -1;

    if (dsbuf) {	/* signals must be deskewed */
	int c, i, j, s;

	/* First, obtain the samples needed. */
	if (dsbi < 0) {	/* dsbuf contents are invalid -- refill dsbuf */
	    for (dsbi = i = 0; i < dsblen; dsbi = i += tspf) {
		stat = getskewedframe(dsbuf + dsbi);
		if (stat < 0)
		    break;
	    }
	    dsbi = 0;
	}
	else {		/* replace oldest frame in dsbuf only */
	    stat = getskewedframe(dsbuf + dsbi);
	    if ((dsbi += tspf) >= dsblen) dsbi = 0;
	}

	/* Assemble the deskewed frame from the data in dsbuf. */
	for (j = s = 0; s < nvsig; s++) {
	    if ((i = j + dsbi + vsd[s]->skew*tspf) >= dsblen) i %= dsblen;
	    for (c = 0; c < vsd[s]->info.spf; c++)
		vector[j++] = dsbuf[i++];
	}
    }
    else		/* no deskewing necessary */
	stat = getskewedframe(vector);
    istime++;
    return (stat);
}

FINT putvec(const WFDB_Sample *vector)
{
    int c, dif, stat = (int)nosig;
    struct osdata *os;
    struct ogdata *og;
    WFDB_Signal s;
    WFDB_Group g;
    WFDB_Sample samp;

    for (s = 0; s < nosig; s++) {
	os = osd[s];
	g = os->info.group;
	og = ogd[os->info.group];
	if (os->info.nsamp++ == (WFDB_Time)0L)
	    os->info.initval = os->samp = *vector;
	for (c = 0; c < os->info.spf; c++, vector++) {
	    /* Replace invalid samples with lowest possible value for format */
	    if ((samp = *vector) == WFDB_INVALID_SAMPLE)
		switch (os->info.fmt) {
		  case 0:
		  case 8:
		  case 16:
		  case 61:
		  case 160:
		  case 516:
		  default:
		    samp = -1 << 15; break;
		  case 80:
		  case 508:
		    samp = -1 << 7; break;
		  case 212:
		    samp = -1 << 11; break;
		  case 310:
		  case 311:
		    samp = -1 << 9; break;
		  case 24:
		  case 524:
		    samp = -1 << 23; break;
		  case 32:
		    samp = -1 << 31; break;
		}
	    switch (os->info.fmt) {
	      case 0:	/* null signal (do not write) */
		os->samp = samp; break;
	      case 8:	/* 8-bit first differences */
	      default:
		/* Handle large slew rates sensibly. */
		if ((dif = samp - os->samp) < -128) { dif = -128; stat=0; }
		else if (dif > 127) { dif = 127; stat = 0; }
		os->samp += dif;
		w8(dif, og);
		break;
	      case 16:	/* 16-bit amplitudes */
		w16(samp, og); os->samp = samp; break;
	      case 61:	/* 16-bit amplitudes, bytes swapped */
		w61(samp, og); os->samp = samp; break;
	      case 80:	/* 8-bit offset binary amplitudes */
		w80(samp, og); os->samp = samp; break;
	      case 160:	/* 16-bit offset binary amplitudes */
		w160(samp, og); os->samp = samp; break;
	      case 212:	/* 2 12-bit amplitudes bit-packed in 3 bytes */
		w212(samp, og); os->samp = samp; break;
	      case 310:	/* 3 10-bit amplitudes bit-packed in 4 bytes */
		w310(samp, og); os->samp = samp; break;
	      case 311:	/* 3 10-bit amplitudes bit-packed in 4 bytes */
		w311(samp, og); os->samp = samp; break;
	      case 24: /* 24-bit amplitudes */
	        w24(samp, og); os->samp = samp; break;
	      case 32: /* 32-bit amplitudes */
	        w32(samp, og); os->samp = samp; break;

	      case 508: /* 8-bit compressed FLAC */
	      case 516:	/* 16-bit compressed FLAC */
	      case 524:	/* 24-bit compressed FLAC */
		if (flac_putsamp(samp, os->info.fmt, og) < 0)
		    stat = -1;
		os->samp = samp;
		break;
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

FINT isigsettime(WFDB_Time t)
{
    WFDB_Group g;
    WFDB_Time curtime;
    int stat = 0;
	
    /* Return immediately if no seek is needed. */
    if (nisig == 0) return (0);
    if (ifreq <= (WFDB_Frequency)0) {
	if (!(gvmode & WFDB_HIGHRES) || ispfmax < 2)
	    curtime = istime;
	else
	    curtime = (istime - 1) * ispfmax + gvc;
	if (t == curtime) return (0);
    }

    for (g = 1; g < nigroup; g++)
        if ((stat = isgsettime(g, t)) < 0) break;
    /* Seek on signal group 0 last (since doing so updates istime and would
       confuse isgsettime if done first). */
    if (stat == 0) stat = isgsettime(0, t);
    return (stat);
}
    
FINT isgsettime(WFDB_Group g, WFDB_Time t)
{
    int spf, stat, trem = 0;
    double tt;

    /* Handle negative arguments as equivalent positive arguments. */
    if (t < 0L) {
	if (t < -WFDB_TIME_MAX) {
	    wfdb_error("isigsettime: improper seek on signal group %d\n", g);
	    return (-1);
	}
	t = -t;
    }

    /* Convert t to raw sample intervals if we are resampling. */
    if (ifreq > (WFDB_Frequency)0) {
	tt = t * sfreq/ifreq;
	if (tt > WFDB_TIME_MAX) {
	    wfdb_error("isigsettime: improper seek on signal group %d\n", g);
	    return (-1);
	}
	t = (WFDB_Time) tt;
    }

    /* If we're in WFDB_HIGHRES mode, convert t from samples to frames, and
       save the remainder (if any) in trem. */
    if (gvmode & WFDB_HIGHRES) {
	trem = t % ispfmax;
	t /= ispfmax;
    }

    /* Mark the contents of the deskewing buffer (if any) as invalid. */
    dsbi = -1;

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

    return (stat);
}

FSITIME tnextvec(WFDB_Signal s, WFDB_Time t)
{
    int stat = 0;
    WFDB_Time tf;

    if (in_msrec && need_sigmap) { /* variable-layout multi-segment record */
	if (s >= nvsig) {
	    wfdb_error("nextvect: illegal signal number %d\n", s);
	    return ((WFDB_Time) -1);
	}
	/* Go to the start (t) if not already there. */
	if (t != istime && isigsettime(t) < 0) return ((WFDB_Time) -1);
	while (stat >= 0) {
	    char *p = vsd[s]->info.desc, *q;
	    int ss;

	    tf = segp->samp0 + segp->nsamp;  /* end of current segment */
	    /* Check if signal s is available in the current segment. */
	    for (ss = 0; ss < nisig; ss++)
		if ((q = isd[ss]->info.desc) && strcmp(p, q) == 0)
		    break;
	    if (ss < nisig) {
		/* The current segment contains the desired signal.
		   Read samples until we find a valid one or reach
		   the end of the segment. */
		for ( ; t <= tf && (stat = getvec(vvector)) > 0; t++)
		    if (vvector[s] != WFDB_INVALID_SAMPLE) {
			isigsettime(t);
			return (t);
		    }
		if (stat < 0) return ((WFDB_Time) -1);
	    }
	    /* Go on to the next segment. */
	    if (t != tf) stat = isigsettime(t = tf);
	}
    }
    else {	/* single-segment or fixed-layout multi-segment record */
	/* Go to the start (t) if not already there. */
	if (t != istime && isigsettime(t) < 0) return ((WFDB_Time) -1);
	if (s >= nisig) {
	    wfdb_error("nextvect: illegal signal number %d\n", s);
	    return ((WFDB_Time) -1);
	}
	for ( ; (stat = getvec(vvector)) > 0; t++)
	    /* Read samples until we find a valid one or reach the end of the
	       record. */
	    if (vvector[s] != WFDB_INVALID_SAMPLE) {
		isigsettime(t);
		return (t);
	    }
    }
    /* Error or end of record without finding another sample of signal s. */
    return ((WFDB_Time) stat);
}  

FINT setibsize(int n)
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

FINT setobsize(int n)
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

FINT newheader(char *record)
{
    int stat;
    WFDB_Signal s;
    WFDB_Siginfo *osi;

    /* Remove trailing .hea, if any, from record name. */
    wfdb_striphea(record);

    SUALLOC(osi, nosig, sizeof(WFDB_Siginfo));
    for (s = 0; s < nosig; s++)
	copysi(&osi[s], &osd[s]->info);
    stat = setheader(record, osi, nosig);
    for (s = 0; s < nosig; s++) {
	SFREE(osi[s].fname);
	SFREE(osi[s].desc);
	SFREE(osi[s].units);
    }
    SFREE(osi);
    return (stat);
}

FINT setheader(char *record, const WFDB_Siginfo *siarray, unsigned int nsig)
{
    char *p;
    WFDB_Signal s;

    /* If another output header file was opened, close it. */
    if (oheader) {
	(void)wfdb_fclose(oheader);
	if (outinfo == oheader) outinfo = NULL;
	oheader = NULL;
    }

    /* Remove trailing .hea, if any, from record name. */
    wfdb_striphea(record);

    /* Quit (with message from wfdb_checkname) if name is illegal. */
    if (wfdb_checkname(record, "record"))
	return (-1);

    /* Try to create the header file. */
    if ((oheader = wfdb_open("hea", record, WFDB_WRITE)) == NULL) {
	wfdb_error("newheader: can't create header for record %s\n", record);
	return (-1);
    }

    /* Write the general information line. */
    (void)wfdb_fprintf(oheader, "%s %d %.12g", record, nsig, ffreq);
    if ((cfreq > 0.0 && cfreq != ffreq) || bcount != 0.0) {
	(void)wfdb_fprintf(oheader, "/%.12g", cfreq);
	if (bcount != 0.0)
	    (void)wfdb_fprintf(oheader, "(%.12g)", bcount);
    }
    (void)wfdb_fprintf(oheader, " %ld", nsig > 0 ? siarray[0].nsamp : 0L);
    if (btime != 0L || bdate != (WFDB_Date)0) {
	if (btime == 0L)
	    (void)wfdb_fprintf(oheader, " 0:00");
        else if (btime % 1000 == 0)
	    (void)wfdb_fprintf(oheader, " %s",
			       ftimstr(btime, 1000.0));
	else
	    (void)wfdb_fprintf(oheader, " %s",
			       fmstimstr(btime, 1000.0));
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
	(void)wfdb_fprintf(oheader, " %.12g", siarray[s].gain);
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

FINT getseginfo(WFDB_Seginfo **sarray)
{
    *sarray = segarray;
    return (segments);
}

FINT setmsheader(char *record, char **segment_name, unsigned int nsegments)
{
    WFDB_Frequency msfreq = 0, mscfreq = 0;
    double msbcount = 0;
    int n, nsig = 0, old_in_msrec = in_msrec;
    WFDB_Time *ns;
    unsigned i;

    isigclose();	/* close any open input signals */

    /* If another output header file was opened, close it. */
    if (oheader) {
	(void)wfdb_fclose(oheader);
	if (outinfo == oheader) outinfo = NULL;
	oheader = NULL;
    }

    /* Remove trailing .hea, if any, from record name. */
    wfdb_striphea(record);

    /* Quit (with message from wfdb_checkname) if name is illegal. */
    if (wfdb_checkname(record, "record"))
	return (-1);

    if (nsegments < 1) {
	wfdb_error("setmsheader: record must contain at least one segment\n");
	return (-1);
    }

    SUALLOC(ns, nsegments, sizeof(WFDB_Time));
    for (i = 0; i < nsegments; i++) {
	if (strlen(segment_name[i]) > WFDB_MAXRNL) {
	    wfdb_error(
	     "setmsheader: `%s' is too long for a segment name in record %s\n",
		     segment_name[i], record);
	    SFREE(ns);
	    return (-2);
	}
	in_msrec = 1;
	nsamples = 0;
	n = readheader(segment_name[i]);
	in_msrec = old_in_msrec;
	if (n < 0) {
	    wfdb_error("setmsheader: can't read segment %s header\n",
		     segment_name[i]);
	    SFREE(ns);
	    return (-3);
	}
	if ((ns[i] = nsamples) <= 0L) {
	    wfdb_error("setmsheader: length of segment %s must be specified\n",
		     segment_name[i]);
	    SFREE(ns);
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
		SFREE(ns);
		return (-4);
	    }
	    if (msfreq != ffreq) {
		wfdb_error(
		   "setmsheader: incorrect sampling frequency in segment %s\n",
			 segment_name[i]);
		SFREE(ns);
		return (-4);
	    }
	    msnsamples += ns[i];
	}
    }

    /* Try to create the header file. */
    if ((oheader = wfdb_open("hea", record, WFDB_WRITE)) == NULL) {
	wfdb_error("setmsheader: can't create header file for record %s\n",
		 record);
	SFREE(ns);
	return (-1);
    }

    /* Write the first line of the master header. */
    (void)wfdb_fprintf(oheader,"%s/%u %d %.12g", record, nsegments, nsig, msfreq);
    if ((mscfreq > 0.0 && mscfreq != msfreq) || msbcount != 0.0) {
	(void)wfdb_fprintf(oheader, "/%.12g", mscfreq);
	if (msbcount != 0.0)
	    (void)wfdb_fprintf(oheader, "(%.12g)", msbcount);
    }
    (void)wfdb_fprintf(oheader, " %"WFDB_Pd_TIME, msnsamples);
    if (msbtime != 0L || msbdate != (WFDB_Date)0) {
        if (msbtime % 1000 == 0)
	    (void)wfdb_fprintf(oheader, " %s",
			       ftimstr(msbtime, 1000.0));
	else
	    (void)wfdb_fprintf(oheader, " %s",
			       fmstimstr(msbtime, 1000.0));
    }
    if (msbdate)
	(void)wfdb_fprintf(oheader, "%s", datstr(msbdate));
    (void)wfdb_fprintf(oheader, "\r\n");

    /* Write a line for each segment. */
    for (i = 0; i < nsegments; i++)
	(void)wfdb_fprintf(oheader, "%s %"WFDB_Pd_TIME"\r\n",
			   segment_name[i], ns[i]);

    SFREE(ns);
    return (0);
}

FINT wfdbgetskew(WFDB_Signal s)
{
    if (s < nvsig)
	return (vsd[s]->skew);
    else
	return (0);
}

/* Careful!!  This function is dangerous, and should be used only to restore
   skews when they have been reset as a side effect of using, e.g., sampfreq */
FVOID wfdbsetiskew(WFDB_Signal s, int skew)
{
    if (s < nvsig && skew >= 0 && skew < dsblen / tspf)
        vsd[s]->skew = skew;
}

/* Note: wfdbsetskew affects *only* the skew to be written by setheader.
   It does not affect how getframe deskews input signals, nor does it
   affect the value returned by wfdbgetskew. */
FVOID wfdbsetskew(WFDB_Signal s, int skew)
{
    if (s < nosig)
	osd[s]->skew = skew;
}

FLONGINT wfdbgetstart(WFDB_Signal s)
{
    if (s < nisig)
        return (igd[vsd[s]->info.group]->start);
    else if (s == 0 && hsd != NULL)
	return (hsd[0]->start);
    else
	return (0L);
}

/* Note: wfdbsetstart affects *only* the byte offset to be written by
   setheader.  It does not affect how isgsettime calculates byte offsets, nor
   does it affect the value returned by wfdbgetstart. */
FVOID wfdbsetstart(WFDB_Signal s, long int bytes)
{
    if (s < nosig)
        ogd[osd[s]->info.group]->start = bytes;
    prolog_bytes = bytes;
}

FINT wfdbputprolog(const char *buf, long int size, WFDB_Signal s)
{
    long int n;
    WFDB_Group g = osd[s]->info.group;

    n = wfdb_fwrite(buf, 1, size, ogd[g]->fp);
    wfdbsetstart(s, n);
    if (n != size)
	wfdb_error("wfdbputprolog: only %ld of %ld bytes written\n", n, size);
    return (n == size ? 0 : -1);
}

/* Create a .info file (or open it for appending) */
FINT setinfo(char *record)
{
    /* Close any previously opened output info file. */
    int stat = wfdb_oinfoclose();

    /* Quit unless a record name has been specified. */
    if (record == NULL) return (stat);

    /* Remove trailing .hea, if any, from record name. */
    wfdb_striphea(record);

    /* Quit (with message from wfdb_checkname) if name is illegal. */
    if (wfdb_checkname(record, "record"))
	return (-1);

    /* Try to create the .info file. */
    if ((outinfo = wfdb_open("info", record, WFDB_APPEND)) == NULL) {
	wfdb_error("setinfo: can't create info file for record %s\n", record);
	return (-1);
    }

    /* Success! */
    return (0);
}

/* Write an info string to the open output .hea or .info file */
FINT putinfo(const char *s)
{
    if (outinfo == NULL) {
	if (oheader) outinfo = oheader;
	else {
	    wfdb_error("putinfo: caller has not specified a record name\n");
	    return (-1);
	}
    }
    (void)wfdb_fprintf(outinfo, "#%s\r\n", s);
    (void)wfdb_fflush(outinfo);
    return (0);
}

/* getinfo: On the first call, read all info strings from the .hea file and (if
available) the .info file for the specified record, and return a pointer to the
first one.  On subsequent calls, return a pointer to the next info string.
Return NULL if there are no more info strings. */

FSTRING getinfo(char *record)
{
    static char buf[256], *p;
    static int i;
    WFDB_FILE *ifile;

    if (record)
	wfdb_freeinfo();

    if (pinfo == NULL) {	/* info for record has not yet been read */
	if (record == NULL && (record = wfdb_getirec()) == NULL) {
	    wfdb_error("getinfo: caller did not specify record name\n");
	    return (NULL);
	}

	if (ninfo) {
	    wfdb_freeinfo();  /* free memory allocated previously to info */
	    ninfo = 0;
	}

	i = 0;
	nimax = 16;	       /* initial allotment of info string pointers */
	SALLOC(pinfo, nimax, sizeof(char *));

	/* Read info from the .hea file, if available (skip for EDF files) */
	if (!isedf) {
	    /* Remove trailing .hea, if any, from record name. */
	    wfdb_striphea(record);
	    if ((ifile = wfdb_open("hea", record, WFDB_READ))) {
		while (wfdb_fgets(buf, 256, ifile))
		    if (*buf != '#') break; /* skip initial comments, if any */
		while (wfdb_fgets(buf, 256, ifile))
		    if (*buf == '#') break; /* skip header content */
		while (*buf) {	/* read and save info */
		    if (*buf == '#') {	    /* skip anything that isn't info */
			p = buf + strlen(buf) - 1;
			if (*p == '\n') *p-- = '\0';
			if (*p == '\r') *p = '\0';
			if (ninfo >= nimax) {
			    int j = nimax;
			    nimax += 16;
			    SREALLOC(pinfo, nimax, sizeof(char *));
			    memset(pinfo + j, 0, (size_t)(16*sizeof(char *)));
			}
			SSTRCPY(pinfo[ninfo], buf+1);
			ninfo++;
		    }
		    if (wfdb_fgets(buf, 256, ifile) == NULL) break;
		}
		wfdb_fclose(ifile);
	    }
	}
	/* Read more info from the .info file, if available */
	if ((ifile = wfdb_open("info", record, WFDB_READ))) {
	    while (wfdb_fgets(buf, 256, ifile)) {
		if (*buf == '#') {
		    p = buf + strlen(buf) - 1;
		    if (*p == '\n') *p-- = '\0';
		    if (*p == '\r') *p = '\0';
		    if (ninfo >= nimax) {
			int j = nimax;
			nimax += 16;
			SREALLOC(pinfo, nimax, sizeof(char *));
			memset(pinfo + j, 0, (size_t)(16*sizeof(char *)));
		    }
		    SSTRCPY(pinfo[ninfo], buf+1);
		    ninfo++;
		}
	    }
	    wfdb_fclose(ifile);
	}
    }
    if (i < ninfo)
	return pinfo[i++];
    else
	return (NULL);
}

FFREQUENCY sampfreq(char *record)
{
    int n;

    /* Remove trailing .hea, if any, from record name. */
    wfdb_striphea(record);

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

FINT setsampfreq(WFDB_Frequency freq)
{
    if (freq >= 0.) {
	sfreq = ffreq = freq;
	if (spfmax == 0) spfmax = 1;
	if ((gvmode & WFDB_HIGHRES) == WFDB_HIGHRES) sfreq *= spfmax;
	return (0);
    }
    wfdb_error("setsampfreq: sampling frequency must not be negative\n");
    return (-1);
}

static char date_string[37];
static char time_string[62];
static WFDB_Date pdays = -1;

#ifndef __STDC__
#ifndef _WINDOWS
typedef long time_t;
#endif
#endif

FINT setbasetime(char *string)
{
    char *p;

    pdays = -1;
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
	btime = fstrtim(time_string, 1000.0);
#endif
	return (0);
    }
    while (*string == ' ') string++;
    if (p = strchr(string, ' '))
        *p++ = '\0';	/* split time and date components */
    btime = fstrtim(string, 1000.0);
    bdate = p ? strdat(p) : (WFDB_Date)0;
    if (btime == 0L && bdate == (WFDB_Date)0 && *string != '[') {
	if (p) *(--p) = ' ';
	wfdb_error("setbasetime: incorrect time format, '%s'\n", string);
	return (-1);
    }
    return (0);
}

/* Convert sample number to string, using the given sampling
   frequency */
static char *ftimstr(WFDB_Time t, WFDB_Frequency f)
{
    char *p;

    p = strtok(fmstimstr(t, f), ".");		 /* discard msec field */
    if (t <= 0L && (btime != 0L || bdate != (WFDB_Date)0)) { /* time of day */
	(void)strcat(p, date_string);		  /* append dd/mm/yyyy */
	(void)strcat(p, "]");
    }
    return (p);	
}

FSTRING timstr(WFDB_Time t)
{
    double f;

    if (ifreq > 0.) f = ifreq;
    else if (sfreq > 0.) f = sfreq;
    else f = 1.0;

    return ftimstr(t, f);
}

/* Convert sample number to string, using the given sampling
   frequency */
static char *fmstimstr(WFDB_Time t, WFDB_Frequency f)
{
    int hours, minutes, seconds, msec;
    WFDB_Date days;
    double tms;
    WFDB_Time s;

    if (t > 0L || (btime == 0L && bdate == (WFDB_Date)0)) { /* time interval */
	if (t < 0L) t = -t;
	/* Convert from sample intervals to seconds. */
	s = (WFDB_Time)(t / f);
	msec = (int)((t - s*f)*1000/f + 0.5);
	if (msec == 1000) { msec = 0; s++; }
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
	/* Convert to milliseconds since midnight. */
	tms = btime - (t * 1000.0 / f);
	/* Convert to seconds. */
	s = (WFDB_Time)(tms / 1000.0);
	msec = (int)((tms - s*1000.0) + 0.5);
	if (msec == 1000) { msec = 0; s++; }
	t = s;
	seconds = t % 60;
	t /= 60;
	minutes = t % 60;
	t /= 60;
	hours = t % 24;
	days = t / 24;
	if (days != pdays) {
	    if (bdate > 0)
		(void)datstr(days + bdate);
	    else if (days == 0)
		date_string[0] = '\0';
	    else
		(void)sprintf(date_string, " %ld", days);
	    pdays = days;
	}
	(void)sprintf(time_string, "[%02d:%02d:%02d.%03d%s]",
		      hours, minutes, seconds, msec, date_string);
    }
    return (time_string);
}

FSTRING mstimstr(WFDB_Time t)
{
    double f;

    if (ifreq > 0.) f = ifreq;
    else if (sfreq > 0.) f = sfreq;
    else f = 1.0;

    return fmstimstr(t, f);
}

FFREQUENCY getcfreq(void)
{
    return (cfreq > 0. ? cfreq : ffreq);
}

FVOID setcfreq(WFDB_Frequency freq)
{
    cfreq = freq;
}

FDOUBLE getbasecount(void)
{
    return (bcount);
}

FVOID setbasecount(double counter)
{
    bcount = counter;
}

/* Convert string to sample number, using the given sampling
   frequency */
static WFDB_Time fstrtim(const char *string, WFDB_Frequency f)
{
    const char *p, *q, *r;
    double x, y, z;
    WFDB_Date days;
    WFDB_Time t;

    while (*string==' ' || *string=='\t' || *string=='\n' || *string=='\r')
	string++;
    switch (*string) {
      case 'c': return (cfreq > 0. ?
			(WFDB_Time)((strtod(string+1, NULL)-bcount)*f/cfreq) :
			strtotime(string+1, NULL, 10));
      case 'e':	return ((in_msrec ? msnsamples : nsamples) * 
		        (((gvmode&WFDB_HIGHRES) == WFDB_HIGHRES) ? spfmax: 1));
      case 'f': return (WFDB_Time)(strtotime(string+1, NULL, 10)*f/ffreq);
      case 'i':	return (WFDB_Time)(istime *
			(ifreq > 0.0 ? (ifreq/sfreq) : 1.0) *
			(((gvmode&WFDB_HIGHRES) == WFDB_HIGHRES) ? ispfmax: 1));
      case 'o':	return (ostime);
      case 's':	return (strtotime(string+1, NULL, 10));
      case '[':	  /* time of day, possibly with date or days since start */
	if ((q = strchr(++string, ']')) == NULL)
	    return ((WFDB_Time)0);	/* '[...': malformed time string */
	if ((p = strchr(string, ' ')) == NULL || p > q)
	    days = (WFDB_Date)0;/* '[hh:mm:ss.sss]': time since midnight only */
	else if ((r = strchr(p+1, '/')) == NULL || r > q)
	    days = (WFDB_Date)strtol(p+1, NULL, 10); /* '[hh:mm:ss.sss d]' */
	else
	    days = strdat(p+1) - bdate; /* '[hh:mm:ss.sss dd/mm/yyyy]' */
        x = fstrtim(string, 1000.0) - btime;
        if (days > 0L) x += (days*(24*60*60*1000.0));
        t = (WFDB_Time)(x * f / 1000.0 + 0.5);
	return (-t);
      default:
	x = strtod(string, NULL);
	if ((p = strchr(string, ':')) == NULL) return ((long)(x*f + 0.5));
	y = strtod(++p, NULL);
	if ((p = strchr(p, ':')) == NULL) return ((long)((60.*x + y)*f + 0.5));
	z = strtod(++p, NULL);
	return ((WFDB_Time)((3600.*x + 60.*y + z)*f + 0.5));
    }
}

FSITIME strtim(const char *string)
{
    double f;

    if (ifreq > 0.) f = ifreq;
    else if (sfreq > 0.) f = sfreq;
    else f = 1.0;

    return fstrtim(string, f);
}

/* The functions datstr and strdat convert between Julian dates (used
   internally) and dd/mm/yyyy format dates.  (Yes, this is overkill for this
   application.  For the astronomically-minded, Julian dates are supposed
   to begin at noon GMT, but these begin at midnight local time.)  They are
   based on similar functions in chapter 1 of "Numerical Recipes", by Press,
   Flannery, Teukolsky, and Vetterling (Cambridge U. Press, 1986). */

FSTRING datstr(WFDB_Date date)
{
    int d, m, y, gcorr, jm, jy;
    WFDB_Date jd;

    if (date >= 2299161L) {	/* Gregorian calendar correction */
	gcorr = (int)(((date - 1867216L) - 0.25)/36524.25);
	date += 1 + gcorr - (long)(0.25*gcorr);
    }
    date += 1524;
    jy = (int)(6680 + ((date - 2439870L) - 122.1)/365.25);
    jd = (WFDB_Date)(365L*jy + (0.25*jy));
    jm = (int)((date - jd)/30.6001);
    d = date - jd - (int)(30.6001*jm);
    if ((m = jm - 1) > 12) m -= 12;
    y = jy - 4715;
    if (m > 2) y--;
    if (y <= 0) y--;
    (void)sprintf(date_string, " %02d/%02d/%d", d, m, y);
    pdays = -1;
    return (date_string);
}

FDATE strdat(const char *string)
{
    const char *mp, *yp;
    int d, m, y, gcorr, jm, jy;
    WFDB_Date date;

    if ((mp = strchr(string,'/')) == NULL || (yp = strchr(mp+1,'/')) == NULL ||
	(d = strtol(string, NULL, 10)) < 1 || d > 31 ||
	(m = strtol(mp+1, NULL, 10)) < 1 || m > 12 ||
	(y = strtol(yp+1, NULL, 10)) == 0)
	return (0L);
    if (m > 2) { jy = y; jm = m + 1; }
    else { jy = y - 1; 	jm = m + 13; }
    if (jy > 0) date = (WFDB_Date)(365.25*jy);
    else date = -(long)(-365.25 * (jy + 0.25));
    date += (int)(30.6001*jm) + d + 1720995L;
    if (d + 31L*(m + 12L*y) >= (15 + 31L*(10 + 12L*1582))) { /* 15/10/1582 */
	gcorr = (int)(0.01*jy);		/* Gregorian calendar correction */
	date += 2 - gcorr + (int)(0.25*gcorr);
    }
    return (date);
}

FINT adumuv(WFDB_Signal s, WFDB_Sample a)
{
    double x;
    WFDB_Gain g = (s < nvsig) ? vsd[s]->info.gain : WFDB_DEFGAIN;

    if (g == 0.) g = WFDB_DEFGAIN;
    x = a*1000./g;
    if (x >= 0.0)
	return ((int)(x + 0.5));
    else
	return ((int)(x - 0.5));
}

FSAMPLE muvadu(WFDB_Signal s, int v)
{
    double x;
    WFDB_Gain g = (s < nvsig) ? vsd[s]->info.gain : WFDB_DEFGAIN;

    if (g == 0.) g = WFDB_DEFGAIN;
    x = g*v*0.001;
    if (x >= 0.0)
	return ((int)(x + 0.5));
    else
	return ((int)(x - 0.5));
}

FDOUBLE aduphys(WFDB_Signal s, WFDB_Sample a)
{
    double b;
    WFDB_Gain g;

    if (s < nvsig) {
	b = vsd[s]->info.baseline;
	g = vsd[s]->info.gain;
	if (g == 0.) g = WFDB_DEFGAIN;
    }
    else {
	b = 0;
	g = WFDB_DEFGAIN;
    }
    return ((a - b)/g);
}

FSAMPLE physadu(WFDB_Signal s, double v)
{
    int b;
    WFDB_Gain g;

    if (s < nvsig) {
	b = vsd[s]->info.baseline;
	g = vsd[s]->info.gain;
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
sample_vflag is true (non-zero) if the requested sample is not beyond the end
of the record, false (zero) otherwise.  The caller must open the input signals
and must set the global variable nisig to the number of input signals before
invoking sample().  Once this has been done, the caller may request samples in
any order. */

#define BUFLN   4096	/* must be a power of 2, see sample() */

FSAMPLE sample(WFDB_Signal s, WFDB_Time t)
{
    static WFDB_Sample v;
    static WFDB_Time tt;
    int nsig = (nvsig > nisig) ? nvsig : nisig;

    /* Allocate the sample buffer on the first call. */
    if (sbuf == NULL) {
	SALLOC(sbuf, nsig, BUFLN*sizeof(WFDB_Sample));
	tt = (WFDB_Time)-1L;
    }

    /* If the caller requested a sample from an unavailable signal, return
       an invalid value.  Note that sample_vflag is not cleared in this
       case.  */
    if (s < 0 || s >= nsig) {
        sample_vflag = -1;
	return (WFDB_INVALID_SAMPLE);
    }

    /* If the caller specified a negative sample number, prepare to return
       sample 0.  This behavior differs from the convention that only the
       absolute value of the sample number matters. */
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
	if (isigsettime(tt+1) < 0) {
	    sample_vflag = 0;
	    return (WFDB_INVALID_SAMPLE);
	}
    }
    /* If the requested sample is not yet in the buffer, read and buffer
       more samples.  If we reach the end of the record, clear sample_vflag
       and return the last valid value. */
    while (t > tt)
        if (getvec(sbuf + nsig * ((++tt)&(BUFLN-1))) < 0) {
	    --tt;
	    sample_vflag = 0;
	    return (*(sbuf + nsig * (tt&(BUFLN-1)) + s));
	}

    /* The requested sample is in the buffer.  Set sample_vflag and
       return the requested sample. */
    if ((v = *(sbuf + nsig * (t&(BUFLN-1)) + s)) == WFDB_INVALID_SAMPLE)
        sample_vflag = -1;
    else
        sample_vflag = 1;
    return (v);
}

FINT sample_valid(void)
{
    return (sample_vflag);
}

/* Private functions (for use by other WFDB library functions only). */

void wfdb_sampquit(void)
{
    if (sbuf) {
	SFREE(sbuf);
	sample_vflag = 0;
    }
}

void wfdb_sigclose(void)
{
    isigclose();
    osigclose();
    btime = bdate = nsamples = msbtime = msbdate = msnsamples = (WFDB_Time)0;
    sfreq = ifreq = ffreq = (WFDB_Frequency)0;
    pdays = (WFDB_Date)-1;
    segments = in_msrec = skewmax = 0;
    if (dsbuf) {
	SFREE(dsbuf);
	dsbi = -1;
    }
    if (segarray) {
	int i;

	SFREE(segarray);
	segp = segend = (WFDB_Seginfo *)NULL;
	for (i = 0; i < maxisig; i++) {
	    SFREE(isd[i]->info.fname);  /* missing before 10.4.6 */
	    SFREE(isd[i]->info.desc);
	    SFREE(isd[i]->info.units);
	}
    }
    SFREE(segarray_L);
    SFREE(gv0);
    SFREE(gv1);
    SFREE(tvector);
    SFREE(uvector);
    SFREE(vvector);
    tuvlen = 0;

    sigmap_cleanup();
}

void wfdb_osflush(void)
{
    WFDB_Group g;
    WFDB_Signal s;
    struct ogdata *og;
    struct osdata *os;

    if (!osd || !ogd)
	return;

    for (s = 0; s < nosig; s++) {
	if ((os = osd[s]) && (og = ogd[os->info.group]) && og->nrewind == 0) {
	    if (!og->force_flush && og->seek == 0) {
		if (og->bsize == 0 && !wfdb_fseek(og->fp, 0L, SEEK_CUR))
		    og->seek = 1;
		else
		    og->seek = -1;
	    }
	    if (og->force_flush || og->seek > 0) {
		/* Either we are closing the file (osigclose() was called),
		   or the file is seekable: write out any
		   partially-completed sets of bit-packed samples. */
		switch (os->info.fmt) {
		  case 212: f212(og); break;
		  case 310: f310(og); break;
		  case 311: f311(og); break;
		  default: break;
		}
	    }
	}
    }

    for (g = 0; g < nogroup; g++) {
	og = ogd[g];
	if (og->bsize == 0 && og->bp != og->buf) {
	    (void)wfdb_fwrite(og->buf, 1, og->bp - og->buf, og->fp);
	    og->bp = og->buf;
	}
	(void)wfdb_fflush(og->fp);

	if (!og->force_flush && og->nrewind != 0) {
	    /* Rewind the file so that subsequent samples will be
	       written in the right place. */
	    wfdb_fseek(og->fp, -((long) og->nrewind), SEEK_CUR);
	    og->nrewind = 0;
	}
    }
}

/* Release resources allocated for info string handling */
void wfdb_freeinfo(void)
{
    int i;

    for (i = 0; i < nimax; i++)
	SFREE(pinfo[i]);
    SFREE(pinfo);
    nimax = ninfo = 0;
}

/* Close any previously opened output info file. */
int wfdb_oinfoclose(void)
{
    int stat = 0, errflag;

    if (outinfo && outinfo != oheader) {
	errflag = wfdb_ferror(outinfo);
	if (wfdb_fclose(outinfo))
	    errflag = 1;
	if (errflag) {
	    wfdb_error("setinfo: write error in info file\n");
	    stat = -2;
	}
    }
    outinfo = NULL;
    return (stat);
}

#ifdef WFDB_LARGETIME

/* Wrapper functions

   The following functions are provided for compatibility with
   applications that do not support WFDB_LARGETIME.  The behavior,
   arguments, and return values of these functions are the same as the
   "non-wrapped" functions above, except that 'long' is used in place
   of WFDB_Time. */

struct WFDB_seginfo_L {
    char recname[WFDB_MAXRNL+1];
    /* WFDB_Time */ long nsamp;
    /* WFDB_Time */ long samp0;
};

static long tclamp(WFDB_Time t)
{
    if (t >= LONG_MIN && t <= LONG_MAX)
	return (t);
    errno = ERANGE;
    return (t < 0 ? LONG_MIN : LONG_MAX);
}

#undef isigsettime
FINT isigsettime(long t)
{
    return (wfdb_isigsettime_LL(t));
}

#undef isgsettime
FINT isgsettime(WFDB_Group g, long t)
{
    return (wfdb_isgsettime_LL(g, t));
}

#undef tnextvec
FLONGINT tnextvec(WFDB_Signal s, long t)
{
    return (tclamp(wfdb_tnextvec_LL(s, t)));
}

#undef timstr
FSTRING timstr(long t)
{
    return (wfdb_timstr_LL(t));
}

#undef mstimstr
FSTRING mstimstr(long t)
{
    return (wfdb_mstimstr_LL(t));
}

#undef strtim
FLONGINT strtim(char *time_string)
{
    return (tclamp(wfdb_strtim_LL(time_string)));
}

#undef sample
FSAMPLE sample(WFDB_Signal s, long t)
{
    return (wfdb_sample_LL(s, t));
}

#undef getseginfo
FINT getseginfo(struct WFDB_seginfo_L **sarray)
{
    int i;
    if (segarray && !segarray_L) {
	SALLOC(segarray_L, segments, sizeof(struct WFDB_seginfo_L));
	for (i = 0; i < segments; i++) {
	    memcpy(segarray_L[i].recname, segarray[i].recname,
		   sizeof(segarray[i].recname));
	    segarray_L[i].nsamp = tclamp(segarray[i].nsamp);
	    segarray_L[i].samp0 = tclamp(segarray[i].samp0);
	}
    }
    *sarray = segarray_L;
    return (segments);
}

#endif /* WFDB_LARGETIME */
