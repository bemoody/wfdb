/* file: signal.c	G. Moody	13 April 1989
			Last revised:   17 July 2001		wfdblib 10.1.6
WFDB library functions for signals

_______________________________________________________________________________
wfdb: a library for reading and writing annotated waveforms (time series data)
Copyright (C) 2000 George B. Moody

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
 isfmt		(checks if argument is a legal signal format type)
 readheader	(reads a header file)
 isigclose	(closes input signals)
 osigclose	(closes output signals)
 isgsetframe	(skips to a specified frame number in a specified signal group)
 getskewedframe	(reads an input frame, without skew correction)

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
 getvec		(reads a sample from each input signal)
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
 sampfreq	(gets the sampling frequency of a record)
 setsampfreq	(sets the sampling frequency)
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

(Numbers in brackets in the list above indicate the first version of the WFDB
library that included the corresponding function.  Functions not so marked
have been included in all published versions of the WFDB library.)

These functions, also defined here, are intended only for the use of WFDB
library functions defined elsewhere:
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
*/

#include "wfdblib.h"

#ifdef iAPX286
#define BROKEN_CC
#endif

#ifndef NOTIME
#include <time.h>
#endif

/* Shared local data */
static unsigned ispfmax;	/* max number of samples of any open signal
				   per input frame */
static unsigned nisig;		/* number of open input signals */
static WFDB_Time istime;	/* time of next input sample */
static int ibsize;		/* default input buffer size */
static char *isbuf[WFDB_MAXSIG];/* pointers to input buffers */
static char *isbp[WFDB_MAXSIG];	/* pointers to next locations in isbuf[] */
static char *isbe[WFDB_MAXSIG];	/* pointers to input buffer endpoints */
static WFDB_Sample ivec[WFDB_MAXSIG]; /* most recent samples read */
static char icount[WFDB_MAXSIG];/* input counters for bit-packed signals */
static WFDB_FILE *isf[WFDB_MAXSIG]; /* file pointers for open input signals */
static WFDB_Siginfo isinfo[WFDB_MAXSIG]; /* input signal information */
static char iseek[WFDB_MAXSIG];	/* flags to indicate if seeks are permitted */
static int istat[WFDB_MAXSIG];	/* signal file status flags */
static long istart[WFDB_MAXSIG];/* signal file byte offsets to sample 0 */
static long ostart[WFDB_MAXSIG];/* byte offsets to be written by setheader() */
static int iskew[WFDB_MAXSIG];	/* intersignal skew (in frames) */
static int oskew[WFDB_MAXSIG];	/* skews to be written by setheader() */
static unsigned skewmax;	/* max skew (frames) between any 2 signals */
static WFDB_Sample *dsbuf;	/* deskewing buffer */
static int dsbi;		/* index to oldest sample in dsbuf (if < 0,
				   dsbuf does not contain valid data) */
static unsigned dsblen;		/* capacity of dsbuf, in samples */
static unsigned framelen;	/* total number of samples per frame */
static int gvmode = WFDB_LOWRES;/* getvec mode (WFDB_HIGHRES or WFDB_LOWRES) */
static int gvc;			/* getvec sample-within-frame counter */

static unsigned nosig;		/* number of open output signals */
static WFDB_Time ostime;	/* time of next output sample */
static int obsize;		/* default output buffer size */
static char *osbuf[WFDB_MAXSIG];/* pointers to output buffers */
static char *osbp[WFDB_MAXSIG];	/* pointers to next locations in osbuf[]; */
static char *osbe[WFDB_MAXSIG];	/* pointers to output buffer endpoints */
static WFDB_Sample ovec[WFDB_MAXSIG]; /* most recent samples written */
static char ocount[WFDB_MAXSIG];/* output counters for bit-packed signals */
static WFDB_FILE *osf[WFDB_MAXSIG]; /* file pointers for open output signals */
static WFDB_Siginfo osinfo[WFDB_MAXSIG]; /* output signal information */

static WFDB_FILE *iheader;	/* file pointer for input header file */
static WFDB_FILE *oheader;	/* file pointer for output header file */
static WFDB_Frequency ffreq;	/* frame rate (frames/second) */
static WFDB_Frequency sfreq;	/* sampling frequency (samples/second) */
static WFDB_Frequency cfreq;	/* counter frequency (ticks/second) */
static WFDB_Time btime;		/* base time (seconds since midnight) */
static WFDB_Date bdate;		/* base date (Julian date) */
static WFDB_Time nsamples;	/* duration of signals (in samples) */
static double bcount;		/* base count (counter value at sample 0) */
static WFDB_Siginfo tsinfo[WFDB_MAXSIG]; /* WFDB_Siginfos for temporary use */
static long tstart[WFDB_MAXSIG];/* signal file byte offsets to sample 0 */

static int in_msrec;		/* current input record is: 0: a single-segment
				   record; 1: a multi-segment record */
static int segments;		/* number of segments in current multi-segment
				   input record (0: single-segment record) */
static WFDB_Time msbtime;	/* base time for multi-segment record */
static WFDB_Date msbdate;	/* base date for multi-segment record */
static WFDB_Time msnsamples;	/* duration of multi-segment record */
static struct segrec {
    char recname[WFDB_MAXRNL+1];/* segment name */
    WFDB_Time nsamp;		/* number of samples in segment */
    WFDB_Time samp0;		/* sample number of first sample in segment */
} *segarray, *segp, *segend;	/* beginning, current segment, end pointers */

/* Local functions (not accessible outside this file). */

static int isfmt(f)
int f;
{
    int i;
    static int fmt_list[WFDB_NFMTS] = WFDB_FMT_LIST;

    for (i = 0; i < WFDB_NFMTS; i++)
	if (f == fmt_list[i]) return (1);
    return (0);
}

static int readheader(record, siarray)
char *record;
WFDB_Siginfo *siarray;
{
    char linebuf[256], *p, *q;
    WFDB_Frequency f;
    WFDB_Signal s;
    WFDB_Time ns;
    unsigned int i, nsig;
    static char sep[] = " \t\n\r";

    /* If another input header file was opened, close it. */
    if (iheader) (void)wfdb_fclose(iheader);

    /* Try to open the header file. */
    if ((iheader = wfdb_open("header", record, WFDB_READ)) == NULL) {
	wfdb_error("init: can't open header for record %s\n", record);
	return (-1);
    }

    /* Get the first token (the record name) from the first non-empty,
       non-comment line. */
    do {
	if (wfdb_fgets(linebuf, 256, iheader) == NULL) {
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
    if (iheader->type == WFDB_LOCAL &&
	iheader->fp != stdin && strcmp(p, record) != 0) {
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
    if (p = strtok((char *)NULL, sep)) {

	/* The file appears to be a new-style header file.  
	   The second token specifies the number of signals. */
	if ((nsig = (unsigned)atoi(p)) > WFDB_MAXSIG) {
	    wfdb_error(
		"init: number of signals in record %s header is incorrect\n",
		     record);
	    return (-2);
	}

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
	    btime == (WFDB_Time)0L && setbasetime(p) < 0)
		return (-2);	/* error message will come from setbasetime */

	/* Special processing for master header of a multi-segment record. */
	if (segments && !in_msrec) {
	    msbtime = btime;
	    msbdate = bdate;
	    msnsamples = nsamples;
	    /* Read the names and lengths of the segment records. */
#ifndef lint
	    segarray = (struct segrec *)
		malloc((unsigned)(sizeof(struct segrec) * segments));
#endif
	    if (segarray == (struct segrec *)NULL) {
		wfdb_error("init: insufficient memory\n");
		segments = 0;
		return (-2);
	    }
	    segp = segarray;
	    for (i = 0, ns = (WFDB_Time)0L; i < segments; i++, segp++) {
		/* Get next segment spec, skip empty lines and comments. */
		do {
		    if (wfdb_fgets(linebuf, 256, iheader) == NULL) {
			wfdb_error(
		         "init: unexpected EOF in header file for record %s\n",
				 record);
#ifndef lint
			(void)free(segarray);
#endif
			segarray = (struct segrec *)NULL;
			segments = 0;
			return (-2);
		    }
		} while ((p = strtok(linebuf, sep)) == NULL || *p == '#');
		if (strlen(p) > WFDB_MAXRNL) {
		    wfdb_error(
		    "init: `%s' is too long for a segment name in record %s\n",
			     p, record);
#ifndef lint
		    (void)free(segarray);
#endif
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
#ifndef lint
		    (void)free(segarray);
#endif
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

	/* Now get information for each signal. */
	for (s = 0; s < nsig; s++) {
	    int nobaseline;

	    /* Get the first token (the signal file name) from the next
	       non-empty, non-comment line. */
	    do {
		if (wfdb_fgets(linebuf, 256, iheader) == NULL) {
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
	    if (s == 0 || strcmp(p, siarray[s-1].fname)) {
		siarray[s].group = (s == 0) ? 0 : siarray[s-1].group + 1;
		if ((siarray[s].fname =
		     (char *)malloc((unsigned)(strlen(p)+1)))
		     == NULL) {
		    wfdb_error("init: insufficient memory\n");
		    return (-2);
		}
		(void)strcpy(siarray[s].fname, p);
	    }
	    /* If the file names of the current and previous signals match,
	       they are assigned the same group number and share a copy of the
	       file name.  All signals associated with a given file must be
	       listed together in the header in order to be identified as
	       belonging to the same group;  readheader does not check that
	       this has been done. */
	    else {
		siarray[s].group = siarray[s-1].group;
		siarray[s].fname = siarray[s-1].fname;
	    }

	    /* Determine the signal format. */
	    if ((p = strtok((char *)NULL, sep)) == NULL ||
		!isfmt(siarray[s].fmt = atoi(p))) {
		wfdb_error("init: illegal format for signal %d, record %s\n",
			 s, record);
		return (-2);
	    }
	    siarray[s].spf = 1;
	    iskew[s] = 0;
	    tstart[s] = 0L;
	    while (*(++p)) {
		if (*p == 'x' && *(++p))
		    if ((siarray[s].spf = atoi(p)) < 1) siarray[s].spf = 1;
		if (*p == ':' && *(++p))
		    if ((iskew[s] = atoi(p)) < 0) iskew[s] = 0;
		if (*p == '+' && *(++p))
		    if ((tstart[s] = atol(p)) < 0L) tstart[s] = 0L;
	    }
	    /* The resolution for deskewing is one frame.  The skew in samples
	       (given in the header) is converted to skew in frames here. */
	    iskew[s] = (int)(((double)iskew[s])/siarray[s].spf + 0.5);

	    /* Determine the gain in ADC units per physical unit.  This number
               may be zero or missing;  if so, the signal is uncalibrated. */
	    if (p = strtok((char *)NULL, sep))
		siarray[s].gain = (WFDB_Gain)atof(p);
	    else
		siarray[s].gain = (WFDB_Gain)0.;

	    /* Determine the baseline if specified, and the physical units
	       (assumed to be millivolts unless otherwise specified). */
	    nobaseline = 1;
	    if (p) {
		for ( ; *p && *p != '(' && *p != '/'; p++)
		    ;
		if (*p == '(') {
		    siarray[s].baseline = atoi(++p);
		    nobaseline = 0;
		}
		while (*p)
		    if (*p++ == '/' && *p)
			break;
	    }
	    if (p && *p) {
		if ((siarray[s].units=(char *)malloc(WFDB_MAXUSL+1)) == NULL) {
		    wfdb_error("init: insufficient memory\n");
		    return (-2);
		}
		(void)strncpy(siarray[s].units, p, WFDB_MAXUSL);
	    }
	    else
		siarray[s].units = NULL;

	    /* Determine the ADC resolution in bits.  If this number is
	       missing and cannot be inferred from the format, the default
	       value (from wfdb.h) is filled in. */
	    if (p = strtok((char *)NULL, sep))
		i = (unsigned)atoi(p);
	    else switch (siarray[s].fmt) {
	      case 80: i = 8; break;
	      case 160: i = 16; break;
	      case 212: i = 12; break;
	      case 310: i = 10; break;
	      case 311: i = 10; break;
	      default: i = WFDB_DEFRES; break;
	    }
	    siarray[s].adcres = i;

	    /* Determine the ADC zero (assumed to be zero if missing). */
	    siarray[s].adczero = (p = strtok((char *)NULL, sep)) ? atoi(p) : 0;
	    
	    /* Set the baseline to adczero if no baseline field was found. */
	    if (nobaseline) siarray[s].baseline = siarray[s].adczero;

	    /* Determine the initial value (assumed to be equal to the ADC 
	       zero if missing). */
	    siarray[s].initval = (p = strtok((char *)NULL, sep)) ?
		                 atoi(p) : siarray[s].adczero;

	    /* Determine the checksum (assumed to be zero if missing). */
	    if (p = strtok((char *)NULL, sep)) {
		siarray[s].cksum = atoi(p);
		siarray[s].nsamp = ns;
	    }
	    else {
		siarray[s].cksum = 0;
		siarray[s].nsamp = (WFDB_Time)0L;
	    }

	    /* Determine the block size (assumed to be zero if missing). */
	    siarray[s].bsize = (p = strtok((char *)NULL, sep)) ? atoi(p) : 0;

	    /* Check that formats and block sizes match for signals belonging
	       to the same group. */
	    if (s && siarray[s].group == siarray[s-1].group &&
		(siarray[s].fmt != siarray[s-1].fmt ||
		 siarray[s].bsize != siarray[s-1].bsize)) {
		wfdb_error("init: error in specification of signal %d or %d\n",
			 s-1, s);
		return (-2);
	    }
	    
	    /* Get the signal description.  If missing, a description of
	       the form "record xx, signal n" is filled in. */
	    if ((siarray[s].desc = (char *)malloc(WFDB_MAXDSL+1)) == NULL) {
		wfdb_error("init: insufficient memory\n");
		return (-2);
	    }
	    if (p = strtok((char *)NULL, "\n\r"))
		(void)strncpy(siarray[s].desc, p, WFDB_MAXDSL);
	    else
		(void)sprintf(siarray[s].desc,
			      "record %s, signal %d", record, s);
	}
    }

    else {
	/* We come here if there were no other tokens on the line which
	   contained the record name.  The file appears to be an old-style
	   header file. */
	wfdb_error("init: obsolete format in record %s header\n", record);
	return (-2);
    }

    return (s);			/* return number of available signals */
}
		
static void isigclose()
{
    while (nisig) {
	if (--nisig == 0 || isinfo[nisig].group != isinfo[nisig-1].group) {
	    (void)wfdb_fclose(isf[nisig]);
	    isf[nisig] = NULL;
	    if (isbuf[isinfo[nisig].group]) {
#ifndef lint
		(void)free(isbuf[isinfo[nisig].group]);
#endif
		isbuf[isinfo[nisig].group] = NULL;
	    }
	    if (isinfo[nisig].fname) {
#ifndef lint
		(void)free(isinfo[nisig].fname);
#endif
		isinfo[nisig].fname = NULL;
	    }
	}
	icount[nisig] = 0;
    }
    istime = 0L;
    gvc = ispfmax = 1;
    if (iheader) {
	(void)wfdb_fclose(iheader);
	iheader = NULL;
    }
}

static void osigclose()
{
    WFDB_Group g;

    while (nosig) {
	if (--nosig == 0 || osinfo[nosig].group != osinfo[nosig-1].group) {
	    g = osinfo[nosig].group;
	    /* Null-pad special files only, if anything was written. */
	    if (osinfo[nosig].bsize != 0 && ocount[nosig])
		while (osbp[g] != osbe[g])
		    *(osbp[g]++) = '\0';
	    /* Flush the last block. */
	    if (osbp[g] != osbuf[g])
		(void)wfdb_fwrite(osbuf[g], 1, osbp[g]-osbuf[g], osf[nosig]);
	    /* Don't close standard output (this will be done on exit). */
	    if (strcmp(osinfo[nosig].fname, "-")) {
		(void)wfdb_fclose(osf[nosig]);
		osf[nosig] = NULL;
	    }
#ifndef lint
	    (void)free(osbuf[g]);
	    (void)free(osinfo[nosig].fname);
#endif
	    osbuf[g] = osinfo[nosig].fname = NULL;
	}
	if (osinfo[nosig].desc) {
#ifndef lint
	    (void)free(osinfo[nosig].desc);
#endif
	    osinfo[nosig].desc = NULL;
	}
	if (osinfo[nosig].units) {
#ifndef lint
	    (void)free(osinfo[nosig].units);
#endif
	    osinfo[nosig].units = NULL;
	}
	ocount[nosig] = 0;
    }
    ostime = 0L;
    if (oheader) {
	(void)wfdb_fclose(oheader);
	oheader = NULL;
    }
}

/* Low-level I/O routines.  The input routines each get a single argument (the
signal number).  The output routines get two arguments (the value to be written
and the signal number). */

static WFDB_Group _g;	    /* macro temporary storage for group number */
static int _l;		    /* macro temporary storage for low byte of word */
static int _n;		    /* macro temporary storage for byte count */

#define r8(S)	((char)(_g = isinfo[S].group, \
		 ((isbp[_g] < isbe[_g]) ? *(isbp[_g]++) : \
		  ((_n = (isinfo[S].bsize > 0) ? isinfo[S].bsize : ibsize), \
		   (istat[_g] = _n = wfdb_fread(isbuf[_g], 1, _n, isf[S])), \
		   (isbe[_g] = (isbp[_g] = isbuf[_g]) + _n),\
		  *(isbp[_g]++)))))

#define w8(V,S)	(_g = osinfo[S].group, \
		 (*(osbp[_g]++) = (char)(V), \
		  (_l = ((osbp[_g] != osbe[_g]) ? 0 : \
		   ((_n = (osinfo[S].bsize > 0) ? osinfo[S].bsize : obsize), \
		    (wfdb_fwrite((osbp[_g] = osbuf[_g]), 1, _n, osf[S])))))))

/* If a short integer is not 16 bits, it may be necessary to redefine r16() and
r61() in order to obtain proper sign extension. */

#ifndef BROKEN_CC
#define r16(S)	(_l = r8(S), ((int)((short)((r8(S) << 8) | (_l & 0xff)))))
#define w16(V,S)	(w8((V), (S)), w8(((V) >> 8), (S)))
#define r61(S)  (_l = r8(S), ((int)((short)((r8(S) & 0xff) | (_l << 8)))))
#define w61(V,S)	(w8(((V) >> 8), (S)), w8((V), (S)))
#else

static int r16(s)
WFDB_Signal s;
{
    int l, h;

    l = r8(s);
    h = r8(s);
    return ((int)((short)((h << 8) | (l & 0xff))));
}

static void w16(v, s)
WFDB_Sample v;
WFDB_Signal s;
{
    w8(v, s);
    w8((v >> 8), s);
}

static int r61(s)
WFDB_Signal s;
{
    int l, h;

    h = r8(s);
    l = r8(s);
    return ((int)((short)((h << 8) | (l & 0xff))));
}

static void w61(v, s)
WFDB_Sample v;
WFDB_Signal s;
{
    w8((v >> 8), s);
    w8(v, s);
}
#endif

#define r80(S)		((r8(S) & 0xff) - (1 << 7))
#define w80(V, S)	(w8(((V) & 0xff) + (1 << 7), S))

#define r160(S)		((r16(S) & 0xffff) - (1 << 15))
#define w160(V, S)	(w16(((V) & 0xffff) + (1 << 15), S))

static int r212(s)  /* read and return the next sample from a format 212    */
WFDB_Signal s;	    /* signal file (2 12-bit samples bit-packed in 3 bytes) */
{
    WFDB_Group g = isinfo[s].group;
    int v;
    static int x[WFDB_MAXSIG];

    /* Obtain the next 12-bit value right-justified in v. */
    switch (icount[g]++) {
      case 0:	v = x[g] = r16(s); break;
      case 1:
      default:	icount[g] = 0;
	        v = ((x[g] >> 4) & 0xf00) | (r8(s) & 0xff); break;
    }
    /* Sign-extend from the twelfth bit. */
    if (v & 0x800) v |= ~(0xfff);
    else v &= 0xfff;
    return (v);
}

static void w212(v, s)	/* write the next sample to a format 212 signal file */
WFDB_Sample v;
WFDB_Signal s;
{
    WFDB_Group g = osinfo[s].group;
    static int x[WFDB_MAXSIG];

    /* Samples are buffered here and written in pairs, as three bytes. */
    switch (ocount[g]++) {
      case 0:	x[g] = v & 0xfff; break;
      case 1:	ocount[g] = 0;
		x[g] |= (v << 4) & 0xf000; w16(x[g], s); w8(v, s);
		break;
    }
}

static int r310(s)  /* read and return the next sample from a format 310    */
WFDB_Signal s;	    /* signal file (3 10-bit samples bit-packed in 4 bytes) */
{
    WFDB_Group g = isinfo[s].group;
    int v;
    static int x[WFDB_MAXSIG], y[WFDB_MAXSIG];

    /* Obtain the next 10-bit value right-justified in v. */
    switch (icount[g]++) {
      case 0:	v = (x[g] = r16(s)) >> 1; break;
      case 1:	v = (y[g] = r16(s)) >> 1; break;
      case 2:
      default:	icount[g] = 0;
		v = ((x[g] & 0xf800) >> 11) | ((y[g] & 0xf800) >> 6); break;
    }
    /* Sign-extend from the tenth bit. */
    if (v & 0x200) v |= ~(0x3ff);
    else v &= 0x3ff;
    return (v);
}

static void w310(v, s)	/* write the next sample to a format 310 signal file */
WFDB_Sample v;
WFDB_Signal s;
{
    WFDB_Group g = osinfo[s].group;
    static int x[WFDB_MAXSIG], y[WFDB_MAXSIG];

    /* Samples are buffered here and written in groups of three, as two
       left-justified 15-bit words. */
    switch (ocount[g]++) {
      case 0:	x[g] = (v << 1) & 0x7fe; break;
      case 1:	y[g] = (v << 1) & 0x7fe; break;
      case 2:
      default:	ocount[g] = 0;
	        x[g] |= (v << 11); w16(x[g], s);
	        y[g] |= ((v <<  6) & ~0x7fe); w16(y[g], s);
		break;
    }
}

/* Note that formats 310 and 311 differ in the layout of the bit-packed data */

static int r311(s)  /* read and return the next sample from a format 311    */
WFDB_Signal s;	    /* signal file (3 10-bit samples bit-packed in 4 bytes) */
{
    WFDB_Group g = isinfo[s].group;
    int v;
    static int x[WFDB_MAXSIG], y[WFDB_MAXSIG];

    /* Obtain the next 10-bit value right-justified in v. */
    switch (icount[g]++) {
      case 0:	v = (x[g] = r16(s)); break;
      case 1:	y[g] = r16(s);
	        v = ((x[g] & 0xfc00) >> 10) | ((y[g] & 0xf) << 6); break;
      case 2:
      default:	icount[g] = 0;
		v = y[g] >> 4; break;
    }
    /* Sign-extend from the tenth bit. */
    if (v & 0x200) v |= ~(0x3ff);
    else v &= 0x3ff;
    return (v);
}

static void w311(v, s)	/* write the next sample to a format 311 signal file */
WFDB_Sample v;
WFDB_Signal s;
{
    WFDB_Group g = osinfo[s].group;
    static int x[WFDB_MAXSIG], y[WFDB_MAXSIG];

    /* Samples are buffered here and written in groups of three, bit-packed
       into the 30 low bits of a 32-bit word. */
    switch (ocount[g]++) {
      case 0:	x[g] = v & 0x3ff; break;
      case 1:	x[g] |= (v << 10); w16(x[g], s);
	        y[g] = (v >> 6) & 0xf; break;
      case 2:
      default:	ocount[g] = 0;
	        y[g] |= (v << 4); y[g] &= 0x3fff; w16(y[g], s);
		break;
    }
}

static int isgsetframe(g, t)
WFDB_Group g;
WFDB_Time t;
{
    int i, trem = 0;
    long nb, tt;
    WFDB_Signal s;
    unsigned int b, d = 1, n, nn;


    /* Find the first signal which belongs to group g. */
    for (s = 0; s < nisig && g != isinfo[s].group; s++)
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
	    if (isigopen(segp->recname, tsinfo, (int)nisig) != nisig) {
	        wfdb_error("isigsettime: can't open segment %s\n",
			   segp->recname);
		return (-1);
	    }
	}
	t -= segp->samp0;
    }

    /* Determine the number of samples per frame for signals in the group. */
    for (n = nn = 0; s+n < nisig && isinfo[s+n].group == g; n++)
	nn += isinfo[s+n].spf;
    /* Determine the number of bytes per sample interval in the file. */
    switch (isinfo[s].fmt) {
      case 0:
	if (t < nsamples) {
	    if (s == 0) istime = (in_msrec) ? t + segp->samp0 : t;
	    isinfo[s].nsamp = nsamples - t;
	    return (istat[g] = 1);
	}
	else {
	    if (s == 0) istime = (in_msrec) ? msnsamples : nsamples;
	    isinfo[s].nsamp = 0L;
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
	icount[g] = 0;
	/* If the desired sample does not lie on a byte boundary, seek to
	   the previous sample and then read ahead. */
	if ((nn & 1) && (t & 1)) {
	    if (in_msrec)
		t += segp->samp0;	/* restore absolute time */
	    if (i = isgsetframe(g, t - 1))
		return (i);
	    for (i = 0; i < nn; i++)
		(void)r212(s);
	    istime++;
	    return (0);
	}
	b = 3*nn; d = 2; break;
      case 310:
	/* Reset the input counter. */
	icount[g] = 0;
	/* If the desired sample does not lie on a byte boundary, seek to
	   the closest previous sample that does, then read ahead. */
	if ((nn % 3) && (trem = (t % 3))) {
	    if (in_msrec)
		t += segp->samp0;	/* restore absolute time */
	    if (i = isgsetframe(g, t - trem))
		return (i);
	    for (i = nn*trem; i > 0; i--)
		(void)r310(s);
	    istime += trem;
	    return (0);
	}		  
	b = 4*nn; d = 3; break;
      case 311:
	/* Reset the input counter. */
	icount[g] = 0;
	/* If the desired sample does not lie on a byte boundary, seek to
	   the closest previous sample that does, then read ahead. */
	if ((nn % 3) && (trem = (t % 3))) {
	    if (in_msrec)
		t += segp->samp0;	/* restore absolute time */
	    if (i = isgsetframe(g, t - trem))
		return (i);
	    for (i = nn*trem; i > 0; i--)
		(void)r311(s);
	    istime += trem;
	    return (0);
	}		  
	b = 4*nn; d = 3; break;
    }

    /* Seek to the beginning of the block which contains the desired sample.
       For normal files, use fseek() to do so. */
    if (iseek[g]) {
	tt = t*b;
	nb = tt/d + istart[s];
	i = (isinfo[s].bsize == 0) ? ibsize : isinfo[s].bsize;
	/* Seek to a position such that the next block read will contain the
	   desired sample. */
	tt = nb/i;
	if (wfdb_fseek(isf[s], tt*i, 0)) {
	    wfdb_error("isigsettime: improper seek on signal group %d\n", g);
	    return (-1);
	}
	nb %= i;
    }
    /* For special files, rewind if necessary and then read ahead. */
    else {
	long t0, t1;

	t0 = istime - (isbp[g] - isbuf[g])/b;	/* earliest buffered sample */
	t1 = t0 + (isbe[g] - isbuf[g])/b;	/* earliest unread sample */
	/* There are three possibilities:  either the desired sample has been
	   read and has passed out of the buffer, requiring a rewind ... */
	if (t < t0) {
	    if (wfdb_fseek(isf[s], 0L, 0)) {
		wfdb_error("isigsettime: improper seek on signal group %d\n",
			   g);
		return (-1);
	    }
	    tt = t*b;
	    nb = tt/d + istart[s];
	}
	/* ... or it is in the buffer already ... */
	else if (t < t1) {
	    tt = (t - t0)*b;
	    isbp[g] = isbuf[g] + tt/d;
	    return (0);
	}
	/* ... or it has not yet been read. */
	else {
	    tt = (t - t1) * b;
	    nb = tt/d;
	}
	while (nb > isinfo[s].bsize && !wfdb_feof(isf[s]))
	    nb -= wfdb_fread(isbuf[g], 1, isinfo[s].bsize, isf[s]);
    }

    /* Reset the block pointer to indicate nothing has been read in the
       current block. */
    isbp[g] = isbe[g];
    istat[g] = 1;
    /* Read any bytes in the current block which precede the desired sample. */
    while (nb-- > 0 && istat[g] > 0)
	i = r8(s);
    if (istat[g] <= 0) return (-1);

    /* Reset the getvec sample-within-frame counter. */
    gvc = ispfmax;

    /* Reset the time (if signal 0 belongs to the group) and disable checksum
       testing (by setting the number of samples remaining to 0). */
    if (s == 0) istime = in_msrec ? t + segp->samp0 : t;
    while (n-- != 0)
	isinfo[s+n].nsamp = (WFDB_Time)0L;
    return (0);
}

static int getskewedframe(vector)
WFDB_Sample *vector;
{
    int c, stat = (int)nisig;
    WFDB_Signal s;

    if (istime == 0L) {
	/* Go through groups in reverse order since seeking on group 0 should
	   always be done last. */
	s = nisig-1;
	do {
	    ivec[s] = isinfo[s].initval;
	    if ((s == 0 || isinfo[s].group != isinfo[s-1].group) &&
		istart[s] > 0L)
		(void)isgsetframe(isinfo[s].group, 0L);
	    
	} while (s-- != 0);
    }

    for (s = 0; s < nisig; s++) {
	for (c = 0; c < isinfo[s].spf; c++, vector++) {
	    switch (isinfo[s].fmt) {
	      case 0:	/* null signal: return adczero */
		*vector = isinfo[s].adczero;
		if (isinfo[s].nsamp == 0) istat[isinfo[s].group] = -1;
		break;
	      case 8:	/* 8-bit first differences */
	      default:
		*vector = ivec[s] += r8(s); break;
	      case 16:	/* 16-bit amplitudes */
		*vector = r16(s); break;
	      case 61:	/* 16-bit amplitudes, bytes swapped */
		*vector = r61(s); break;
	      case 80:	/* 8-bit offset binary amplitudes */
		*vector = r80(s); break;
	      case 160:	/* 16-bit offset binary amplitudes */
		*vector = r160(s); break;
	      case 212:	/* 2 12-bit amplitudes bit-packed in 3 bytes */
		*vector = r212(s); break;
	      case 310:	/* 3 10-bit amplitudes bit-packed in 4 bytes */
		*vector = r310(s); break;
	      case 311:	/* 3 10-bit amplitudes bit-packed in 4 bytes */
		*vector = r311(s); break;
	    }
	    if (istat[isinfo[s].group] <= 0) {
		/* End of file -- reset input counter. */
		icount[isinfo[s].group] = 0;
		if (isinfo[s].nsamp > (WFDB_Time)0L) {
		    wfdb_error("getvec: unexpected EOF in signal %d\n", s);
		    stat = -3;
		}
		else if (in_msrec && segp < segend) {
		    segp++;
		    if (isigopen(segp->recname, tsinfo, (int)nisig) < nisig) {
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
		if (isinfo[s].nsamp > (WFDB_Time)0L) {
		    wfdb_error("getvec: unexpected EOF in signal %d\n", s);
		    stat = -3;
		}
		else
		    stat = -1;
	    }
	    else
		isinfo[s].cksum -= *vector;
	}
	if (--isinfo[s].nsamp == (WFDB_Time)0L && (isinfo[s].cksum & 0xffff) &&
	    isinfo[s].fmt != 0) {
	    wfdb_error("getvec: checksum error in signal %d\n", s);
	    stat = -4;
	}
    }
    return (stat);
}

/* WFDB library functions. */

FINT isigopen(record, siarray, nsig)
char *record;
WFDB_Siginfo *siarray;
int nsig;
{
    WFDB_FILE **ifp;
    WFDB_Group g;
    int navail;
    WFDB_Siginfo *isi;
    WFDB_Signal s, si, sj;
    unsigned int buflen, ga;

    /* Close previously opened input signals unless otherwise requested. */
    if (*record == '+') record++;
    else isigclose();

    /* Save the current record name. */
    if (!in_msrec) wfdb_setirec(record);

    /* Read the header and determine how many signals are available. */
    if ((navail = readheader(record, tsinfo)) <= 0) {
	if (navail == 0 && segments) {	/* this is a multi-segment record */
	    in_msrec = 1;
	    /* Open the first segment to get signal information. */
	    if ((navail = readheader(segp->recname, tsinfo)) >= 0) {
		if (msbtime == 0L) msbtime = btime;
		if (msbdate == 0L) msbdate = bdate;
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
	for (s = 0; s < nsig; s++)
	    siarray[s] = tsinfo[s];
	return (navail);
    }

    /* Initialize local variables. */
    ifp = &isf[nisig];
    isi = &isinfo[nisig];
    if (ibsize <= 0) ibsize = BUFSIZ;

    /* Determine how many signals we should attempt to open.  The caller's
       upper limit on this number is nsig, the upper limit defined by the
       header is navail, and the upper limit imposed by the WFDB library is
       WFDB_MAXSIG-nisig (the number of empty slots in the input signal
       arrays). */
    if (nsig > navail) nsig = navail;
    if (nsig > WFDB_MAXSIG - nisig) nsig = WFDB_MAXSIG - nisig;

    /* Set the group number adjustment.  This quantity is added to the group
       numbers of signals which are opened below;  it accounts for any input
       signals which were left open from previous calls. */
    ga = nisig ? isinfo[nisig-1].group + 1 : 0;

    /* Open the signal files.  One signal group is handled per iteration.  In
       this loop, si counts through the entries in tsinfo, and s counts the
       entries added to the isinfo and isf (ifp) arrays. */
    for (si = s = 0; si < navail && s < nsig; ) {
	/* Find out how many signals are in this group. */
	for (sj = si+1; sj<navail && tsinfo[sj].group==tsinfo[si].group; sj++)
	    ;
	/* Skip this group if not enough slots are available. */
	if (sj-si > nsig-s) {
	    si = sj;
	    ga--;
	    continue;
	}
	/* Don't open a file for a null signal. */
	if (tsinfo[si].fmt == 0)
	    ifp[s] = NULL;
	/* The file name '-' specifies the standard input. */
	else if (strcmp(tsinfo[si].fname, "-") == 0) {
	    ifp[s]->type = WFDB_LOCAL;
	    ifp[s]->fp = stdin;
	}
	/* Skip this group if the signal file can't be opened. */
	else if ((ifp[s]=wfdb_open(tsinfo[si].fname,(char *)NULL,WFDB_READ)) ==
		 NULL){
	    si = sj;
	    ga--;
	    continue;
	}
	g = tsinfo[si].group + ga;  /* the next group number to be assigned */
	if (tsinfo[si].bsize > 0) buflen = tsinfo[si].bsize;
	else if (tsinfo[si].bsize < 0) buflen = -tsinfo[si].bsize;
	else buflen = ibsize;
	/* Skip this group if a buffer can't be allocated. */
	if ((isbuf[g] = (char *)malloc(buflen)) == NULL) {
	    si = sj;
	    ga--;
	    continue;
	}
	isbe[g] = isbp[g] = isbuf[g] + buflen;
	istat[g] = 1;
	if (tsinfo[si].bsize < 0) {
	    tsinfo[si].bsize = -tsinfo[si].bsize;
	    iseek[g] = 0;
	}
	else
	    iseek[g] = 1;
	istart[s+nisig] = tstart[si];
	isi[s] = tsinfo[si++];
	isi[s++].group = g;
	while (si < sj) {
	    ifp[s] = ifp[s-1];
	    istart[s+nisig] = tstart[si];
	    isi[s] = tsinfo[si++];
	    isi[s++].group = g;
	}
    }

    /* Produce a warning message if none of the requested signals could be
       opened. */
    if (s == 0 && nsig)
	wfdb_error("isigopen: none of the signals for record %s is readable\n",
		 record);

    /* Copy the WFDB_Siginfo structures to the caller's array. */
    for (si = 0; si < s; si++) {
	siarray[si] = isi[si];
	if (ispfmax < isi[si].spf) ispfmax = isi[si].spf;
	if (skewmax < iskew[si]) skewmax = iskew[si];
	ivec[si] = isi[si].initval;	/* *** check this *** */
    }

    /* Reset sfreq if appropriate. */
    setgvmode(gvmode);

    /* Initialize getvec's sample-within-frame counter. */
    gvc = ispfmax;

    /* Update the count of open input signals. */
    nisig += s;

    /* Determine the total number of samples per frame. */
    for (si = framelen = 0; si < nisig; si++)
	framelen += isinfo[si].spf;

    /* If deskewing is required, allocate the deskewing buffer (unless this is
       a multi-segment record and dsbuf has been allocated already); produce a
       warning message if the buffer couldn't be allocated.  In this case, the
       signals can still be read, but won't be deskewed. */
    if (skewmax != 0 && (!in_msrec || dsbuf == NULL)) {
	dsbi = -1;	/* mark buffer contents as invalid */
	dsblen = framelen * (skewmax + 1);
#ifndef lint
	if (dsbuf) free(dsbuf);
	if ((dsbuf=(WFDB_Sample *)malloc(dsblen*sizeof(WFDB_Sample))) == NULL)
	    wfdb_error("isigopen: can't allocate buffer for deskewing\n");
#endif
    }
    return (s);
}

FINT osigopen(record, siarray, nsig)
char *record;
WFDB_Siginfo *siarray;
unsigned int nsig;
{
    WFDB_FILE **ofp;
    int n;
    WFDB_Siginfo *osi;
    WFDB_Signal s, si;
    unsigned int buflen, ga;

    /* Close previously opened output signals unless otherwise requested. */
    if (*record == '+') record++;
    else osigclose();

    /* Determine if nsig output signals can be opened. */
    if (nsig > WFDB_MAXSIG - nosig) {
	wfdb_error("osigopen: attempt to open too many output signals\n");
	return (-3);
    }
    if ((n = readheader(record, tsinfo)) < 0)
	return (n);
    if (n < nsig) {
	wfdb_error("osigopen: record %s has fewer signals than needed\n",
		 record);
	return (-3);
    }

    /* Initialize local variables. */
    ofp = &osf[nosig];
    osi = &osinfo[nosig];
    if (obsize <= 0) obsize = BUFSIZ;

    /* Set the group number adjustment.  This quantity is added to the group
       numbers of signals which are opened below;  it accounts for any output
       signals which were left open from previous calls. */
    ga = nosig ? osinfo[nosig-1].group + 1 : 0;

    /* Open the signal files.  One signal is handled per iteration. */
    for (s = 0; s < nsig; s++) {
	/* Check if this signal is in the same group as the previous one. */
	if (s != 0 && tsinfo[s].group == tsinfo[s-1].group)
	    ofp[s] = ofp[s-1];
	else if (tsinfo[s].fmt == 0)
	    ofp[s] = NULL;	/* don't open a file for a null signal */
	/* The filename '-' specifies the standard output. */
	else if (strcmp(tsinfo[s].fname, "-") == 0) {
	    ofp[s]->type = WFDB_LOCAL;
	    ofp[s]->fp = stdout;
	}
	/* An error in opening an output file is fatal. */
	else if ((ofp[s]=wfdb_open(tsinfo[s].fname,(char *)NULL,WFDB_WRITE)) ==
		 NULL){
	    wfdb_error("osigopen: can't open %s\n", tsinfo[s].fname);
	    return (-3);
	}
	osi[s] = tsinfo[s];
	buflen = (osi[s].bsize > 0) ? osi[s].bsize : obsize;
	osi[s].group += ga;
	/* An error allocating a buffer is fatal. */
	if ((s == 0 || osi[s].group != osi[s-1].group) &&
	    (osbuf[osi[s].group] = (char *)malloc(buflen)) == NULL) {
	    wfdb_error("osigopen: can't allocate buffer for %s\n",
		       osi[s].fname);
	    return (-3);
	}
	osbp[osi[s].group] = osbuf[osi[s].group];
	osbe[osi[s].group] = osbuf[osi[s].group] + buflen;
    }

    /* Copy the WFDB_Siginfo structures to the caller's array. */
    for (si = 0; si < s; si++) {
	osi[si].cksum = 0;
	osi[si].nsamp = (WFDB_Time)0L;
	siarray[si] = osi[si];
    }

    /* Update the count of open output signals. */
    nosig += s;

    return (s);
}

FINT osigfopen(siarray, nsig)
WFDB_Siginfo *siarray;
unsigned int nsig;
{
    unsigned int buflen;

    /* Close any open output signals. */
    osigclose();

    if (obsize <= 0) obsize = BUFSIZ;

    /* Open the signal files.  One signal is handled per iteration. */
    for ( ; nosig < nsig; nosig++) {
	/* Check signal specifications.  The combined lengths of the fname
	   and desc strings should be 80 characters or less, the bsize field
	   must not be negative, the format should be legal, group numbers
	   should be the same if and only if file names are the same, and
	   group numbers should begin at zero and increase in steps of 1. */
	if (strlen(siarray[nosig].fname) + strlen(siarray[nosig].desc) > 80 ||
	    siarray[nosig].bsize < 0 || !isfmt(siarray[nosig].fmt)) {
	    wfdb_error("osigfopen: error in specification of signal %d\n",nosig);
	    return (-2);
	}
	if (!((nosig == 0 && siarray[nosig].group == 0) ||
	    (nosig && siarray[nosig].group == siarray[nosig-1].group &&
	     strcmp(siarray[nosig].fname, siarray[nosig-1].fname) == 0) ||
	    (nosig && siarray[nosig].group == siarray[nosig-1].group + 1 &&
	     strcmp(siarray[nosig].fname, siarray[nosig-1].fname) != 0))) {
	    wfdb_error(
		"osigfopen: incorrect file name or group for signal %d\n",
		     nosig);
	    return (-2);
	}
	/* Check if this signal is in the same group as the previous one. */
	if (nosig && siarray[nosig].group == siarray[nosig-1].group) {
	    osf[nosig] = osf[nosig-1];
	    if (siarray[nosig].fmt != siarray[nosig-1].fmt ||
		siarray[nosig].bsize != siarray[nosig-1].bsize) {
		wfdb_error(
		    "osigfopen: error in specification of signal %d or %d\n",
		     nosig-1, nosig);
		return (-2);
	    }
	}
	else if (siarray[nosig].fmt == 0)
	    osf[nosig] = NULL;	/* don't open a file for a null signal */
	/* The filename '-' specifies the standard output. */
	else if (strcmp(siarray[nosig].fname, "-") == 0) {
	    osf[nosig]->type = WFDB_LOCAL;
	    osf[nosig]->fp = stdout;
	}
	/* An error in opening an output file is fatal. */
	else if ((osf[nosig] = wfdb_open(siarray[nosig].fname, (char *)NULL,
					 WFDB_WRITE)) == NULL) {
	    wfdb_error("osigfopen: can't open %s\n", siarray[nosig].fname);
	    return (-3);
	}
	osinfo[nosig] = siarray[nosig];
	if (osinfo[nosig].spf < 1) osinfo[nosig].spf = 1;
	osinfo[nosig].cksum = 0;
	osinfo[nosig].nsamp = (WFDB_Time)0L;
	buflen = (osinfo[nosig].bsize > 0) ? osinfo[nosig].bsize : obsize;
	/* An error allocating a buffer is fatal. */
	if ((nosig == 0 || siarray[nosig].group != siarray[nosig-1].group) &&
	    (osbuf[osinfo[nosig].group] = (char *)malloc(buflen)) == NULL ||
	    (osinfo[nosig].fname =
	     (char *)malloc((unsigned)strlen(siarray[nosig].fname)+1))==NULL) {
	    wfdb_error("osigfopen: can't allocate buffer for %s\n",
		     osinfo[nosig].fname);
	    return (-3);
	}
	if (nosig != 0 && osinfo[nosig].group == osinfo[nosig-1].group)
	    osinfo[nosig].fname = osinfo[nosig-1].fname;
	else	
	    (void)strcpy(osinfo[nosig].fname, siarray[nosig].fname);
	if ((osinfo[nosig].desc =
	     (char *)malloc((unsigned)strlen(siarray[nosig].desc)+1))==NULL) {
	    wfdb_error("osigfopen: insufficient memory\n");
	    return (-3);
	}
	(void)strcpy(osinfo[nosig].desc, siarray[nosig].desc);
	if (siarray[nosig].units) {
	    if ((osinfo[nosig].units =
		 (char *)malloc((unsigned)strlen(siarray[nosig].units)+1)) ==
		NULL) {
		wfdb_error("osigfopen: insufficient memory\n");
		return (-3);
	    }
	    (void)strcpy(osinfo[nosig].units, siarray[nosig].units);
	}
	osbp[osinfo[nosig].group] = osbuf[osinfo[nosig].group];
	osbe[osinfo[nosig].group] = osbuf[osinfo[nosig].group] + buflen;
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
getvec's operating mode, which is WFDB_LOWRES by default.  When reading
ordinary records (with all signals sampled at the same frequency), getvec's
behavior is independent of its operating mode. */

FINT getspf()
{
    return ((sfreq != ffreq) ? (int)(sfreq/ffreq) : 1);
}

FVOID setgvmode(mode)
int mode;
{
    if (mode == WFDB_HIGHRES) {
	gvmode = WFDB_HIGHRES;
	sfreq = ffreq * ispfmax;
    }
    else {
	gvmode = WFDB_LOWRES;
	sfreq = ffreq;
    }
}

FINT getvec(vector)
WFDB_Sample *vector;
{
    static WFDB_Sample tvector[WFDB_MAXSIG*WFDB_MAXSPF];
    WFDB_Sample *tp;
    WFDB_Signal s;
    static int stat;

    if (ispfmax < 2)	/* all signals at the same frequency */
	return (getframe(vector));

    if (gvmode == WFDB_LOWRES) {/* return one sample per frame, decimating
				   (by averaging) if necessary */
	unsigned c;
	long v;

	stat = getframe(tvector);
	for (s = 0, tp = tvector; s < nisig; s++) {
	    for (c = v = 0; c < isinfo[s].spf; c++)
		v += *tp++;
	    *vector++ = v/isinfo[s].spf;
	}
    }
    else {			/* return ispfmax samples per frame, using
				   zero-order interpolation if necessary */
	if (gvc >= ispfmax) {
	    stat = getframe(tvector);
	    gvc = 0;
	}
	for (s = 0, tp = tvector; s < nisig; s++) {
	    *vector++ = tp[(isinfo[s].spf*gvc)/ispfmax];
	    tp += isinfo[s].spf;
	}
	gvc++;
    }
    return (stat);
}

FINT getframe(vector)
WFDB_Sample *vector;
{
    int stat;

    if (dsbuf) {	/* signals must be deskewed */
	int c, i, j, s;

	/* First, obtain the samples needed. */
	if (dsbi < 0) {	/* dsbuf contents are invalid -- refill dsbuf */
	    for (dsbi = 0; dsbi < dsblen; dsbi += framelen)
		stat = getskewedframe(dsbuf + dsbi);
	    dsbi = 0;
	}
	else {		/* replace oldest frame in dsbuf only */
	    stat = getskewedframe(dsbuf + dsbi);
	    if ((dsbi += framelen) >= dsblen) dsbi = 0;
	}
	/* Assemble the deskewed frame from the data in dsbuf. */
	for (j = s = 0; s < nisig; s++) {
	    if ((i = j + dsbi + iskew[s]*framelen) >= dsblen) i -= dsblen;
	    for (c = 0; c < isinfo[s].spf; c++)
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
    WFDB_Signal s;

    for (s = 0; s < nosig; s++) {
	if (osinfo[s].nsamp++ == (WFDB_Time)0L)
	    osinfo[s].initval = ovec[s] = *vector;
	for (c = 0; c < osinfo[s].spf; c++, vector++) {
	    switch (osinfo[s].fmt) {
	      case 0:	/* null signal (do not write) */
		ovec[s] = *vector; break;
	      case 8:	/* 8-bit first differences */
	      default:
		/* Handle large slew rates sensibly. */
		if ((dif = *vector - ovec[s]) < -128) { dif = -128; stat = 0; }
		else if (dif > 127) { dif = 127; stat = 0; }
		ovec[s] += dif;
		w8(dif, s);
		break;
	      case 16:	/* 16-bit amplitudes */
		w16(*vector, s); ovec[s] = *vector; break;
	      case 61:	/* 16-bit amplitudes, bytes swapped */
		w61(*vector, s); ovec[s] = *vector; break;
	      case 80:	/* 8-bit offset binary amplitudes */
		w80(*vector, s); ovec[s] = *vector; break;
	      case 160:	/* 16-bit offset binary amplitudes */
		w160(*vector, s); ovec[s] = *vector; break;
	      case 212:	/* 2 12-bit amplitudes bit-packed in 3 bytes */
		w212(*vector, s); ovec[s] = *vector; break;
	      case 310:	/* 3 10-bit amplitudes bit-packed in 4 bytes */
		w310(*vector, s); ovec[s] = *vector; break;
	      case 311:	/* 3 10-bit amplitudes bit-packed in 4 bytes */
		w311(*vector, s); ovec[s] = *vector; break;
	    }
	    if (wfdb_ferror(osf[s])) {
		wfdb_error("putvec: write error in signal %d\n", s);
		stat = -1;
	    }
	    else
		osinfo[s].cksum += ovec[s];
	}
    }
    ostime++;
    return (stat);
}

FINT isigsettime(t)
WFDB_Time t;
{
    WFDB_Group g;
    int stat;
    WFDB_Signal s;
	
    /* Return immediately if no seek is needed. */
    if (t == istime || nisig == 0) return (0);

    for (g = s = stat = 0; s < nisig; s++)
	if (isinfo[s].group != g &&
	    (stat = isgsettime(g = isinfo[s].group, t)) < 0) break;
    /* Seek on signal group 0 last (since doing so updates istime and would
       confuse isgsettime if done first). */
    if (stat == 0) stat = isgsettime(0, t);
    return (stat);
}
    
FINT isgsettime(g, t)
WFDB_Group g;
WFDB_Time t;
{
    int spf, stat, trem = 0;

    /* Handle negative arguments as equivalent positive arguments. */
    if (t < 0L) t = -t;

    /* If we're in WFDB_HIGHRES mode, convert t from samples to frames, and
       save the remainder (if any) in trem. */
    if (sfreq != ffreq) {
	spf = (int)(sfreq/ffreq);
	trem = t % spf;
	t /= spf;
    }

    if ((stat = isgsetframe(g, t)) == 0 && g == 0) {
	while (trem-- > 0) {
	    int v[WFDB_MAXSIG*WFDB_MAXSPF];

	    if (getvec(v) < 0) {
		wfdb_error("isigsettime: improper seek on signal group %d\n",
			   g);
		return (-1);
	    }
	}
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
    WFDB_Signal s;

    return (setheader(record, osinfo, nosig));
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
    (void)wfdb_fprintf(oheader, " %ld", siarray[0].nsamp);
    if (btime || bdate)
	(void)wfdb_fprintf(oheader, " %s", timstr((WFDB_Time)(btime*sfreq)));
    if (bdate)
	(void)wfdb_fprintf(oheader, "%s", datstr(bdate));
    (void)wfdb_fprintf(oheader, "\r\n");

    /* Write a signal specification line for each signal. */
    for (s = 0; s < nsig; s++) {
	(void)wfdb_fprintf(oheader, "%s %d", siarray[s].fname, siarray[s].fmt);
	if (siarray[s].spf > 1)
	    (void)wfdb_fprintf(oheader, "x%d", siarray[s].spf);
	if (oskew[s])
	    (void)wfdb_fprintf(oheader, ":%d", oskew[s]*siarray[s].spf);
	if (ostart[s])
	    (void)wfdb_fprintf(oheader, "+%ld", ostart[s]);
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

#ifndef lint
    if ((ns = (long *)malloc((unsigned)(sizeof(long)*nsegments))) == NULL) {
	wfdb_error("setmsheader: insufficient memory\n");
	return (-2);
    }
#endif

    for (i = 0; i < nsegments; i++) {
	if (strlen(segment_name[i]) > WFDB_MAXRNL) {
	    wfdb_error(
	     "setmsheader: `%s' is too long for a segment name in record %s\n",
		     segment_name[i], record);
#ifndef lint
	    (void)free(ns);
#endif
	    return (-2);
	}
	in_msrec = 1;
	n = readheader(segment_name[i], tsinfo);
	in_msrec = old_in_msrec;
	if (n < 0) {
	    wfdb_error("setmsheader: can't read segment %s header\n",
		     segment_name[i]);
#ifndef lint
	    (void)free(ns);
#endif
	    return (-3);
	}
	if ((ns[i] = tsinfo[0].nsamp) <= 0L) {
	    wfdb_error("setmsheader: length of segment %s must be specified\n",
		     segment_name[i]);
#ifndef lint
	    (void)free(ns);
#endif
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
#ifndef lint
		(void)free(ns);
#endif
		return (-4);
	    }
	    if (msfreq != ffreq) {
		wfdb_error(
		   "setmsheader: incorrect sampling frequency in segment %s\n",
			 segment_name[i]);
#ifndef lint
		(void)free(ns);
#endif
		return (-4);
	    }
	    msnsamples += ns[i];
	}
    }

    /* Try to create the header file. */
    if ((oheader = wfdb_open("header", record, WFDB_WRITE)) == NULL) {
	wfdb_error("setmsheader: can't create header file for record %s\n",
		 record);
#ifndef lint
	(void)free(ns);
#endif
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
    if (msbtime || msbdate)
	(void)wfdb_fprintf(oheader, " %s", timstr((WFDB_Time)(msbtime*sfreq)));
    if (msbdate)
	(void)wfdb_fprintf(oheader, "%s", datstr(msbdate));
    (void)wfdb_fprintf(oheader, "\r\n");

    /* Write a line for each segment. */
    for (i = 0; i < nsegments; i++)
	(void)wfdb_fprintf(oheader, "%s %ld\r\n", segment_name[i], ns[i]);

#ifndef lint
    (void)free(ns);
#endif
    return (0);
}

FINT wfdbgetskew(s)
WFDB_Signal s;
{
    if (s < WFDB_MAXSIG)
	return (iskew[s]);
    else
	return (0);
}

/* Careful!!  This function is dangerous, and should be used only to restore
   skews when they have been reset as a side effect of using, e.g., sampfreq */
FVOID wfdbsetiskew(s, skew)
WFDB_Signal s;
int skew;
{
    if (s < WFDB_MAXSIG)
        iskew[s] = skew;
}

/* Note: wfdbsetskew affects *only* the skew to be written by setheader.
   It does not affect how getframe deskews input signals, nor does it
   affect the value returned by wfdbgetskew. */
FVOID wfdbsetskew(s, skew)
WFDB_Signal s;
int skew;
{
    if (s < WFDB_MAXSIG)
	oskew[s] = skew;
}

FLONGINT wfdbgetstart(s)
WFDB_Signal s;
{
    if (s < WFDB_MAXSIG)
	return (istart[s]);
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
    if (s < WFDB_MAXSIG)
	ostart[s] = bytes;
}

FSTRING getinfo(record)
char *record;
{
    char *p;
    static char linebuf[256];

    if (record != NULL && readheader(record, tsinfo) < 0) {
	wfdb_error("getinfo: can't read record %s header\n", record);
	return (NULL);
    }
    else if (record == NULL && iheader == NULL) {
	wfdb_error("getinfo: caller did not specify record name\n");
	return (NULL);
    }

    /* Find a line beginning with '#'. */
    do {
	if (wfdb_fgets(linebuf, 256, iheader) == NULL)
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
    return (0);
}

FFREQUENCY sampfreq(record)
char *record;
{
    int n;

    if (record != NULL) {
	/* Save the current record name. */
	wfdb_setirec(record);
	if ((n = readheader(record, tsinfo)) < 0)
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

FINT setbasetime(string)
char *string;
{
    char *p;

    if (string == NULL || *string == '\0') {
#ifndef NOTIME
	struct tm *now;
#if defined(__STDC__) || defined(_WINDOWS)
	time_t t, time();

	t = time((time_t *)NULL);    /* get current time from system clock */
#else
	long t, time();

	t = time((long *)NULL);
#endif
	now = localtime(&t);
	(void)sprintf(date_string, "%02d/%02d/%d",
		now->tm_mday, now->tm_mon+1, now->tm_year+1900);
	bdate = strdat(date_string);
	(void)sprintf(time_string, "%d:%d:%d",
		now->tm_hour, now->tm_min, now->tm_sec);
	btime = (WFDB_Time)(strtim(time_string)/sfreq);
#endif
	return (0);
    }
    while (*string == ' ') string++;
    if (p = strchr(string, ' '))
        *p++ = '\0';	/* split time and date components */
    if ((btime = strtim(string)) == 0L ||
	(p && (bdate = strdat(p)) == 0L)) {
	if (p) *(--p) = ' ';
	wfdb_error("setbasetime: incorrect time format, '%s'\n", string);
	return (-1);
    }
    btime = (WFDB_Time)(btime/sfreq);
    return (0);
}

FSTRING timstr(t)
WFDB_Time t;
{
    char *p;

    p = strtok(mstimstr(t), ".");		 /* discard msec field */
    if (t <= 0L && (btime != 0L || bdate != 0L)) {	/* time of day */
	(void)strcat(p, date_string);		  /* append dd/mm/yyyy */
	(void)strcat(p, "]");
    }
    return (p);	
}

static WFDB_Date pdays = -1;

FSTRING mstimstr(t)
WFDB_Time t;
{
    double f = (sfreq > 0.) ? sfreq : 1.;
    int hours, minutes, seconds, msec;
    WFDB_Date days;
    long s;

    if (t > 0L || (btime == 0L && bdate == 0L)) {	/* time interval */
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
	t = (WFDB_Time)(btime*sfreq) - t;
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

    f = (sfreq > 0.) ? sfreq : 1.;
    while (*string==' ' || *string=='\t' || *string=='\n' || *string=='\r')
	string++;
    switch (*string) {
      case 'c': return (cfreq > 0. ?
			(WFDB_Time)((atof(string+1)-bcount)*sfreq/cfreq) :
			(WFDB_Time)atol(string+1));
      case 'e':	return (in_msrec ? msnsamples : nsamples);
      case 'f': return ((WFDB_Time)(atol(string+1)*sfreq/ffreq));
      case 'i':	return (istime * (gvmode==WFDB_LOWRES ? 1: ispfmax));
      case 'o':	return (ostime);
      case 's':	return ((WFDB_Time)atol(string+1));
      case '[':	  /* time of day, possibly with date or days since start */
	if (p = strchr(string, ' ')) {
	    if (strchr(p, '/')) days = strdat(p) - bdate;
	    else days = atol(p);
	}
	t = strtim(string+1) - (WFDB_Time)(btime*sfreq);
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
    WFDB_Gain g = (s < nisig) ? isinfo[s].gain : WFDB_DEFGAIN;

    if (g == 0.) g = WFDB_DEFGAIN;
    return ((int)(a*1000./g + 0.5));
}

FSAMPLE muvadu(s, v)
WFDB_Signal s;
int v;
{
    WFDB_Gain g = (s < nisig) ? isinfo[s].gain : WFDB_DEFGAIN;

    if (g == 0.) g = WFDB_DEFGAIN;
    return ((int)(g*v*0.001 + 0.5));
}

FDOUBLE aduphys(s, a)
WFDB_Signal s;
WFDB_Sample a;
{
    int b;
    WFDB_Gain g;

    if (s < nisig) {
	b = isinfo[s].baseline;
	g = isinfo[s].gain;
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
	b = isinfo[s].baseline;
	g = isinfo[s].gain;
	if (g == 0.) g = WFDB_DEFGAIN;
    }
    else {
	b = 0;
	g = WFDB_DEFGAIN;
    }
    return ((int)(g*v + 0.5) + b);
}

/* Private functions (for use by other WFDB library functions only). */

void wfdb_sigclose()
{
    isigclose();
    osigclose();
    btime = bdate = nsamples = msbtime = msbdate = msnsamples = (WFDB_Time)0;
    sfreq = ffreq = (WFDB_Frequency)0;
    pdays = (WFDB_Date)-1;
    segments = in_msrec = skewmax = 0;
    if (dsbuf) {
#ifndef lint
	(void)free(dsbuf);
#endif
	dsbuf = NULL;
	dsbi = -1;
    }
    if (segarray) {
	int i;

#ifndef lint
	(void)free(segarray);
#endif
	segarray = segp = segend = (struct segrec *)NULL;
	for (i = 0; i < WFDB_MAXSIG; i++) {
	    if (isinfo[i].desc) {
#ifndef lint
		(void)free(isinfo[i].desc);
#endif
		isinfo[i].desc = NULL;
	    }
	    if (isinfo[i].units) {
#ifndef lint
		(void)free(isinfo[i].units);
#endif
		isinfo[i].units = NULL;
	    }
	}
    }
}

void wfdb_osflush()
{
    WFDB_Signal s;
    WFDB_Group g;

    for (s = 0; s < nosig; s++)
	if (s == 0 || (g = osinfo[s].group) != osinfo[s-1].group) {
	    if (osinfo[s].bsize == 0 && osbp[g] != osbuf[g]) {
		(void)wfdb_fwrite(osbuf[g], 1, osbp[g] - osbuf[g], osf[s]);
		osbp[g] = osbuf[g];
	    }
	    (void)wfdb_fflush(osf[s]);
	}
}
