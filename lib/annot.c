/* file: annot.c	G. Moody       	 13 April 1989
			Last revised:    14 November 2004	wfdblib 10.3.14
WFDB library functions for annotations

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
 get_ann_table  (reads tables used by annstr, strann, and anndesc)
 put_ann_table  (writes tables used by annstr, strann, and anndesc)
 allociann	(sets max number of simultaneously open input annotators)
 allocoann	(sets max number of simultaneously open output annotators)

This file also contains definitions of the following WFDB library functions:
 annopen        (opens annotation files)
 getann         (reads an annotation)
 ungetann [5.3] (pushes an annotation back into an input stream)
 putann         (writes an annotation)
 iannsettime    (skips to a specified time in input annotation files)
 ecgstr         (converts MIT annotation codes to ASCII strings)
 strecg         (converts ASCII strings to MIT annotation codes)
 setecgstr      (modifies code-to-string translation table)
 annstr [5.3]   (converts user-defined annotation codes to ASCII strings)
 strann [5.3]   (converts ASCII strings to user-defined annotation codes)
 setannstr [5.3](modifies code-to-string translation table)
 anndesc [5.3]  (converts user-defined annotation codes to text descriptions)
 setanndesc [5.3](modifies code-to-text translation table)
 iannclose [9.1](closes an input annotation file)
 oannclose [9.1](closes an output annotation file)

(Numbers in brackets in the list above indicate the first version of the WFDB
library that included the corresponding function.  Functions not so marked
have been included in all published versions of the WFDB library.)

These functions, also defined here, are intended only for the use of WFDB
library functions defined elsewhere:
 wfdb_anclose     (closes all annotation files)
 wfdb_oaflush     (flushes output annotations)

Beginning with version 5.3, the functions in this file read and write
annotation translation table modifications as `modification labels' (`NOTE'
annotations attached to sample 0 of signal 0).  This feature provides
transparent support for custom annotation definitions in WFDB applications.
Previous versions of these functions, if used to read files containing
modification labels, treat them as ordinary NOTE annotations.

Simultaneous annotations attached to different signals (as indicated by the
`chan' field) are supported by version 6.1 and later versions.  Annotations
must be written in time order; simultaneous annotations must be written in
`chan' order.  Simultaneous annotations are readable but not writeable by
earlier versions.
*/

#include "wfdblib.h"
#include "ecgcodes.h"
#define isqrs
#define map1
#define map2
#define annpos
#include "ecgmap.h"

/* Annotation word format */
#define CODE	0176000	/* annotation code segment of annotation word */
#define CS	10	/* number of places by which code must be shifted */
#define DATA	01777	/* data segment of annotation word */
#define MAXRR	01777	/* longest interval which can be coded in a word */

/* Pseudo-annotation codes.  Legal pseudo-annotation codes are between PAMIN
   (defined below) and CODE (defined above).  PAMIN must be greater than
   ACMAX << CS (see <ecg/ecgcodes.h>). */
#define PAMIN	((unsigned)(59 << CS))
#define SKIP	((unsigned)(59 << CS))	/* long null annotation */
#define NUM	((unsigned)(60 << CS))	/* change 'num' field */
#define SUB	((unsigned)(61 << CS))	/* subtype */
#define CHN	((unsigned)(62 << CS))	/* change 'chan' field */
#define AUX	((unsigned)(63 << CS))	/* auxiliary information */

/* Constants for AHA annotation files only */
#define ABLKSIZ	1024		/* AHA annotation file block length */
#define AUXLEN	6		/* length of AHA aux field */
#define EOAF	0377		/* padding for end of AHA annotation files */

/* Shared local data */
static unsigned maxiann;	/* max allowed number of input annotators */
static unsigned niaf;		/* number of open input annotators */
static struct iadata {
    WFDB_FILE *file;		/* file pointer for input annotation file */
    WFDB_Anninfo info;	   	/* input annotator information */
    WFDB_Annotation ann;	/* next annotation to be returned by getann */
    WFDB_Annotation pann; 	/* pushed-back annotation from ungetann */
    unsigned word;		/* next word from the input file */
    int ateof;			/* EOF-reached indicator */
    char auxstr[1+255+1];	/* aux string buffer (byte count+data+null) */
    unsigned index;		/* next available position in auxstr */
    WFDB_Time tt;		/* annotation time (MIT format only).  This
				   equals ann.time unless a SKIP follows ann;
				   in such cases, it is the time of the SKIP
				   (i.e., the time of the annotation following
				   ann) */
} **iad;

static unsigned maxoann;	/* max allowed number of output annotators */
static unsigned noaf;		/* number of open output annotators */
static struct oadata {
    WFDB_FILE *file;		/* file pointer for output annotation file */
    WFDB_Anninfo info;		/* output annotator information */
    WFDB_Annotation ann;	/* most recent annotation written by putann */
    int seqno;			/* annotation serial number (AHA format only)*/
    char *rname;		/* record with which annotator is associated */
    char out_of_order;		/* if >0, one or more annotations written by
				   putann are not in the canonical (time, chan)
				   order */
} **oad;

static double tmul;		/* `time' fields in annotations are
				   tmul * times in annotation files */

/* Local functions (for the use of other functions in this module only). */

static int get_ann_table(i)
WFDB_Annotator i;
{
    char *p1, *p2, *s1, *s2;
    int a;
    WFDB_Annotation annot;

    if (getann(i, &annot) < 0)	/* prime the pump */
	return (-1);
    while (getann(i,&annot) == 0 && annot.time == 0L &&
	   annot.anntyp == NOTE && annot.subtyp == 0) {
	if (annot.aux == NULL || *annot.aux < 1 || *(annot.aux+1) == '#')
	    continue;
	p1 = strtok(annot.aux+1, " \t");
	a = atoi(p1);
	if (0 <= a && a <= ACMAX && (p1 = strtok((char *)NULL, " \t"))) {
	    p2 = p1 + strlen(p1) + 1;
	    if ((s1 = (char *)malloc(((unsigned)(strlen(p1) + 1)))) == NULL ||
		(*p2 &&
		 (s2 = (char *)malloc(((unsigned)(strlen(p2)+1)))) == NULL)) {
		wfdb_error("annopen: insufficient memory\n");
		return (-1);
	    }
	    (void)strcpy(s1, p1);
	    (void)setannstr(a, s1);
	    if (*p2) {
		(void)strcpy(s2, p2);
		(void)setanndesc(a, s2);
	    }
	    else
		(void)setanndesc(a, (char*)NULL);
	}

    }
    if (annot.time != 0L || annot.anntyp != NOTE || annot.subtyp != 0 ||
	annot.aux == NULL ||
	strncmp(annot.aux + 1, "## sampling frequency: ", 23))
	(void)ungetann(i, &annot);
    return (0);
}

static char modified[ACMAX+1];	/* modified[i] is non-zero if setannstr() or
				   setanndesc() has modified the mnemonic or
				   description for annotation type i */   

static int put_ann_table(i)
WFDB_Annotator i;
{
    int a, flag = 0;
    char buf[256];
    WFDB_Annotation annot;

    annot.time = 0L;
    annot.anntyp = NOTE;
    annot.subtyp = annot.chan = annot.num = 0;
    annot.aux = buf;
    for (a = 0; a <= ACMAX; a++)
	if (modified[a]) {
	    if (flag == 0) { /* mark the beginning of the table */
		(void)sprintf(buf+1, "## annotation type definitions");
		buf[0] = strlen(buf+1);
		if (putann(i, &annot) < 0) return (-1);
	    }
	    (void)sprintf(buf+1, "%d %s %s", a, annstr(a), anndesc(a));
	    buf[0] = strlen(buf+1);
	    if (putann(i, &annot) < 0) return (-1);
	    flag = 1;
	}
    if (flag) {	/* if a table was written, mark its end */
	(void)sprintf(buf+1, "## end of definitions");
	buf[0] = strlen(buf+1);
	if (putann(i, &annot) < 0) return (-1);
    }
    return (0);
}

/* Allocate workspace for up to n input annotators. */
static int allociann(n)
unsigned n;
{
    if (maxiann < n) {     /* allocate input annotator data structures */
        unsigned m = maxiann;
        struct iadata **iadnew = realloc(iad, n*sizeof(struct iadata *));

	if (iadnew == NULL) {
	    wfdb_error("annopen: too many (%d) input annotators\n", n);
	    return (-1);
	}
	iad = iadnew;
	while (m < n) {
	    if ((iad[m] = calloc(1, sizeof(struct iadata))) == NULL) {
		wfdb_error("annopen: too many (%d) input annotators\n", n);
		while (--m > maxiann)
		    free(iad[m]);
		return (-1);
	    }
	    m++;
	}
        maxiann = n;
    }
    return (maxiann);
}

/* Allocate workspace for up to n output annotators. */
static int allocoann(n)
unsigned n;
{
    if (maxoann < n) {     /* allocate output annotator data structures */
        unsigned m = maxoann;
        struct oadata **oadnew = realloc(oad, n*sizeof(struct oadata *));

	if (oadnew == NULL) {
	    wfdb_error("annopen: too many (%d) output annotators\n", n);
	    return (-1);
	}
	oad = oadnew;
	while (m < n) {
	    if ((oad[m] = calloc(1, sizeof(struct oadata))) == NULL) {
		wfdb_error("annopen: too many (%d) output annotators\n", n);
		while (--m > maxoann)
		    free(oad[m]);
		return (-1);
	    }
	    m++;
	}
        maxoann = n;
    }
    return (maxoann);
}
    
/* WFDB library functions (for general use). */

FINT annopen(record, aiarray, nann)
char *record;
WFDB_Anninfo *aiarray;
unsigned int nann;
{
    int a;
    unsigned int i, niafneeded, noafneeded;

    if (*record == '+')		/* don't close open annotation files */
	record++;		/* discard the '+' prefix */
    else {
	wfdb_anclose();		/* close previously opened annotation files */
	tmul = 0.0;
    }

    /* Prescan aiarray to see how large maxiann and maxoann must be. */
    niafneeded = niaf;
    noafneeded = noaf;
    for (i = 0; i < nann; i++)
	switch (aiarray[i].stat) {
	  case WFDB_READ:	/* standard (MIT-format) input file */
	  case WFDB_AHA_READ:	/* AHA-format input file */
	    niafneeded++;
	    break;
	  case WFDB_WRITE:	/* standard (MIT-format) output file */
	  case WFDB_AHA_WRITE:	/* AHA-format output file */
	    noafneeded++;
	    break;
	  default:
	    wfdb_error(
		     "annopen: illegal stat %d for annotator %s, record %s\n",
		     aiarray[i].stat, aiarray[i].name, record);
	    return (-5);
	}
    /* Allocate workspace. */
    if (allociann(niafneeded) < 0 || allocoann(noafneeded) < 0)
	return (-3);

    for (i = 0; i < nann; i++) { /* open the annotation files */
	struct iadata *ia;
	struct oadata *oa;

	switch (aiarray[i].stat) {
	  case WFDB_READ:	/* standard (MIT-format) input file */
	  case WFDB_AHA_READ:	/* AHA-format input file */
	    ia = iad[niaf];
	    wfdb_setirec(record);
	    if ((ia->file=wfdb_open(aiarray[i].name,record,WFDB_READ)) ==
		NULL) {
		wfdb_error("annopen: can't read annotator %s for record %s\n",
			 aiarray[i].name, record);
		return (-3);
	    }
	    if ((ia->info.name =
		 (char *)malloc((unsigned)(strlen(aiarray[i].name)+1)))
		 == NULL) {
		wfdb_error("annopen: insufficient memory\n");
		return (-3);
	    }
	    (void)strcpy(ia->info.name, aiarray[i].name);

	    /* Try to figure out what format the file is in.  AHA-format files
	       begin with a null byte and an ASCII character which is one
	       of the legal AHA annotation codes other than '[' or ']'.
	       MIT annotation files cannot begin in this way. */
	    ia->word = (unsigned)wfdb_g16(iad[niaf]->file);
	    a = (ia->word >> 8) & 0xff;
	    if ((ia->word & 0xff) ||
		ammap(a) == NOTQRS || a == '[' || a == ']') {
		if (aiarray[i].stat != WFDB_READ) {
		    wfdb_error("warning (annopen, annotator %s, record %s):\n",
			     aiarray[i].name, record);
		    wfdb_error(" file appears to be in MIT format\n");
		    wfdb_error(" ... continuing under that assumption\n");
		}
		(ia->info).stat = WFDB_READ;
		/* read any initial null annotation(s) */
		while ((ia->word & CODE) == SKIP) {
		    ia->tt += wfdb_g32(ia->file);
		    ia->word = (unsigned)wfdb_g16(ia->file);
		}
	    }
	    else {
		if (aiarray[i].stat != WFDB_AHA_READ) {
		    wfdb_error("warning (annopen, annotator %s, record %s):\n",
			     aiarray[i].name, record);
		    wfdb_error(" file appears to be in AHA format\n");
		    wfdb_error(" ... continuing under that assumption\n");
		}
		ia->info.stat = WFDB_AHA_READ;
	    }
	    ia->ann.anntyp = 0;    /* any pushed-back annot is invalid */
	    niaf++;
	    (void)get_ann_table(niaf-1);
	    break;

	  case WFDB_WRITE:	/* standard (MIT-format) output file */
	  case WFDB_AHA_WRITE:	/* AHA-format output file */
	    oa = oad[noaf];
	    /* Quit (with message from wfdb_checkname) if name is illegal */
	    if (wfdb_checkname(aiarray[i].name, "annotator"))
		return (-4);
	    if ((oa->file=wfdb_open(aiarray[i].name,record,WFDB_WRITE)) ==
		NULL) {
		wfdb_error("annopen: can't write annotator %s for record %s\n",
			 aiarray[i].name, record);
		return (-4);
	    }
	    if ((oa->info.name =
		 (char *)malloc((unsigned)(strlen(aiarray[i].name)+1)))
		== NULL) {
		wfdb_error("annopen: insufficient memory\n");
		return (-4);
	    }
	    (void)strcpy(oa->info.name, aiarray[i].name);
	    if ((oa->rname =
		 (char *)malloc((unsigned)(strlen(record)+1)))
		== NULL) {
		wfdb_error("annopen: insufficient memory\n");
		return (-4);
	    }
	    (void)strcpy(oa->rname, record);
	    oa->ann.time = 0L;
	    oa->info.stat = aiarray[i].stat;
	    oa->out_of_order = 0;
	    (void)put_ann_table(noaf++);
	    break;
	}
    }
    return (0);
}

FINT getann(n, annot)	/* read an annotation from annotator n into *annot */
WFDB_Annotator n;		/* annotator number */
WFDB_Annotation *annot;		/* address of structure to be filled in */
{
    int a, len;
    struct iadata *ia;

    if (n >= niaf || (ia = iad[n]) == NULL || ia->file == NULL) {
	wfdb_error("getann: can't read annotator %d\n", n);
	return (-2);
    }

    if (ia->pann.anntyp) {
	*annot = ia->pann;
	ia->pann.anntyp = 0;
	return (0);
    }

    if (ia->ateof) {
	if (ia->ateof != -1)
	    return (-1);	/* reached logical EOF */
	wfdb_error("getann: unexpected EOF in annotator %s\n",
		 ia->info.name);
	return (-3);
    }
    *annot = ia->ann;

    switch (ia->info.stat) {
      case WFDB_READ:		/* MIT-format input file */
      default:
	if (ia->word == 0) {	/* logical end of file */
	    ia->ateof = 1;
	    return (0);
	}
	ia->tt += ia->word & DATA; /* annotation time */
	if (ia->tt > 0L && tmul <= 0.0) {
	    WFDB_Frequency f = sampfreq(NULL);

	    tmul = getspf();
	    if (f != (WFDB_Frequency)0)
		tmul = tmul * getifreq() / f;
	}
	ia->ann.time = (WFDB_Time)(ia->tt * tmul + 0.5);
	ia->ann.anntyp = (ia->word & CODE) >> CS; /* set annotation type */
	ia->ann.subtyp = 0;	/* reset subtype field */
	ia->ann.aux = NULL;	/* reset aux field */
	while (((ia->word = (unsigned)wfdb_g16(ia->file))&CODE) >= PAMIN &&
	       !wfdb_feof(ia->file))
	    switch (ia->word & CODE) { /* process pseudo-annotations */
	      case SKIP:  ia->tt += wfdb_g32(ia->file); break;
	      case SUB:   ia->ann.subtyp = DATA & ia->word; break;
	      case CHN:   ia->ann.chan = DATA & ia->word; break;
	      case NUM:	  ia->ann.num = DATA & ia->word; break;
	      case AUX:			/* auxiliary information */
		len = ia->word & 0377;	/* length of auxiliary data */
		if (ia->index >= 256 - len)
		    ia->index = 0;	/* buffer index */
		ia->ann.aux = ia->auxstr + ia->index;    /* save pointer */
		ia->auxstr[ia->index++] = len;	/* save length byte */
		/* Now read the data.  Note that an extra byte may be
		   present in the annotation file to preserve word alignment;
		   if so, this extra byte is read and then overwritten by
		   the null in the second statement below. */
		(void)wfdb_fread(ia->auxstr+ia->index,1,(len+1)&~1,ia->file);
		ia->auxstr[ia->index + len] = '\0';	      /* add a null */
		ia->index += len+1;		     /* update buffer index */
		break;
	      default: break;
	    }
	break;
      case WFDB_AHA_READ:		/* AHA-format input file */
	if ((ia->word&0377) == EOAF) { /* logical end of file */
	    ia->ateof = 1;
	    return (0);
	}
	a = ia->word >> 8;		 /* AHA annotation code */
	ia->ann.anntyp = ammap(a);	 /* convert to MIT annotation code */
	if (tmul <= 0.0) {
	    WFDB_Frequency f = sampfreq(NULL);

	    tmul = getspf();
	    if (f != (WFDB_Frequency)0)
		tmul = tmul * getifreq() / f;
	}
	ia->ann.time = (WFDB_Time)(wfdb_g32(ia->file) * tmul + 0.5);
					 /* time of annotation */
	if (wfdb_g16(ia->file) <= 0)	 /* serial number (starts at 1) */
	    wfdb_error("getann: unexpected annot number in annotator %s\n",
		       ia->info.name);
	ia->ann.subtyp = wfdb_getc(ia->file); /* MIT annotation subtype */
	if (a == 'U' && ia->ann.subtyp == 0)
	    ia->ann.subtyp = -1;	 /* unreadable (noise subtype -1) */
	ia->ann.chan = wfdb_getc(ia->file);	 /* MIT annotation code */
	if (ia->index >= 256 - (AUXLEN+2)) ia->index = 0;
	/* read aux data */
	(void)wfdb_fread(ia->auxstr + ia->index + 1, 1, AUXLEN, ia->file);
	/* There is very limited space in AHA format files for auxiliary
	   information, so no length byte is recorded;  instead, we
	   assume that if the first byte of auxiliary data is
	   not null, that up to AUXLEN bytes may be significant. */
	if (ia->auxstr[ia->index+1]) {
	    ia->auxstr[ia->index] = AUXLEN;
	    ia->ann.aux = ia->auxstr + ia->index; /* save buffer pointer */
	    ia->auxstr[ia->index + 1 + AUXLEN] = '\0';   /* add a null */
	    ia->index += AUXLEN+2;		 /* update buffer index */
	}
	else
	    ia->ann.aux = NULL;
	ia->word = (unsigned)wfdb_g16(ia->file);
	break;
    }
    if (wfdb_feof(ia->file))
	ia->ateof = -1;
    return (0);
}

FINT ungetann(n, annot)   /* push back an annotation into an input stream */
WFDB_Annotator n;		/* annotator number */
WFDB_Annotation *annot;		/* address of annotation to be pushed back */
{
    if (n >= niaf || iad[n] == NULL) {
	wfdb_error("ungetann: annotator %d is not initialized\n", n);
	return (-2);
    }
    if (iad[n]->pann.anntyp) {
	wfdb_error("ungetann: pushback buffer is full\n");
	wfdb_error(
		 "ungetann: annotation at %ld, annotator %d not pushed back\n",
		 annot->time, n);
	return (-1);
    }
    iad[n]->pann = *annot;
    return (0);
}

FINT putann(n, annot)    /* write annotation at annot to annotator n */
WFDB_Annotator n;		/* annotator number */
WFDB_Annotation *annot;		/* address of annotation to be written */
{
    unsigned annwd;
    char *ap;
    int i, len;
    long delta;
    WFDB_Time t;
    struct oadata *oa;

    if (n >= noaf || (oa = oad[n]) == NULL || oa->file == NULL) {
	wfdb_error("putann: can't write annotation file %d\n", n);
	return (-2);
    }
    if (annot->time == 0L)
	t = 0L;
    else {
	if (tmul <= 0.0) {
	    WFDB_Frequency f = sampfreq(NULL);

	    tmul = getspf();
	    if (f != (WFDB_Frequency)0)
		tmul = tmul * getifreq() / f;
	}
	t = (WFDB_Time)(annot->time / tmul + 0.5);
    }
    if (((delta = t - oa->ann.time) < 0L ||
	(delta == 0L && annot->chan <= oa->ann.chan)) &&
	(t != 0L || oa->ann.time != 0L)) {
	oa->out_of_order = 1;
    }
    switch (oa->info.stat) {
      case WFDB_WRITE:	/* MIT-format output file */
      default:
	if (annot->anntyp == 0) {
	    /* The caller intends to write a null annotation here, but putann
	       must not write a word of zeroes that would be interpreted as
	       an EOF.  To avoid this, putann writes a SKIP to the location
	       just before the desired one;  thus annwd (below) is never 0. */
	    wfdb_p16(SKIP, oa->file); wfdb_p32(delta-1, oa->file); delta = 1;
	}
	else if (delta > MAXRR || delta < 0L) {
	    wfdb_p16(SKIP, oa->file); wfdb_p32(delta, oa->file); delta = 0;
	}	
	annwd = (int)delta + ((int)(annot->anntyp) << CS);
	wfdb_p16(annwd, oa->file);
	if (annot->subtyp != 0) {
	    annwd = SUB + (DATA & annot->subtyp);
	    wfdb_p16(annwd, oa->file);
	}
	if (annot->chan != oa->ann.chan) {
	    annwd = CHN + (DATA & annot->chan);
	    wfdb_p16(annwd, oa->file);
	}
	if (annot->num != oa->ann.num) {
	    annwd = NUM + (DATA & annot->num);
	    wfdb_p16(annwd, oa->file);
	}
	if (annot->aux != NULL && *annot->aux != 0) {
	    annwd = AUX+(unsigned)(*annot->aux); 
	    wfdb_p16(annwd, oa->file);
	    (void)wfdb_fwrite(annot->aux + 1, 1, *annot->aux, oa->file);
	    if (*annot->aux & 1)
		(void)wfdb_putc('\0', oa->file);
	}
	break;
      case WFDB_AHA_WRITE:	/* AHA-format output file */
	(void)wfdb_putc('\0', oa->file);
	(void)wfdb_putc(mamap(annot->anntyp, annot->subtyp), oa->file);
	wfdb_p32(t, oa->file);
	wfdb_p16((unsigned int)(++(oa->seqno)), oa->file);
	(void)wfdb_putc(annot->subtyp, oa->file);
	(void)wfdb_putc(annot->anntyp, oa->file);
	if (ap = annot->aux)
	    len = (*ap < AUXLEN) ? *ap : AUXLEN;
	else
	    len = 0;
	for (i = 0, ap++; i < len; i++, ap++)
	    (void)wfdb_putc(*ap, oa->file);
	for ( ; i < AUXLEN; i++)
	    (void)wfdb_putc('\0', oa->file);
	break;
    }
    if (wfdb_ferror(oa->file)) {
	wfdb_error("putann: write error on annotation file %s\n",
		   oa->info.name);
	return (-1);
    }
    oa->ann = *annot;
    oa->ann.time = t;
    return (0);
}

FINT iannsettime(t)
WFDB_Time t;
{
    int stat = 0;
    WFDB_Annotation tempann;
    WFDB_Annotator i;

    /* Handle negative arguments as equivalent positive arguments. */
    if (t < 0L) t = -t;

    for (i = 0; i < niaf && stat == 0; i++) {
        struct iadata *ia;

	ia = iad[i];
	if (ia->ann.time >= t) {	/* "rewind" the annotation file */
	    ia->pann.anntyp = 0;	/* flush pushback buffer */
	    if (wfdb_fseek(ia->file, 0L, 0) == -1) {
		wfdb_error("iannsettime: improper seek\n");
		return (-1);
	    }
	    ia->ann.subtyp = ia->ann.chan = ia->ann.num = ia->ateof = 0;
	    ia->ann.time = ia->tt = 0L;
	    ia->word = wfdb_g16(ia->file);
	    if (ia->info.stat == WFDB_READ)
		while ((ia->word & CODE) == SKIP) {
		    ia->tt += wfdb_g32(ia->file);
		    ia->word = wfdb_g16(ia->file);
		}
	    (void)getann(i, &tempann);
	}
	while (ia->ann.time < t && (stat = getann(i, &tempann)) == 0)
	    ;
    }
    return (stat);
}

static char *cstring[ACMAX+1] = {  /* ECG mnemonics for each code */
        " ",	"N",	"L",	"R",	"a",		/* 0 - 4 */
	"V",	"F",	"J",	"A",	"S",		/* 5 - 9 */
	"E",	"j",	"/",	"Q",	"~",		/* 10 - 14 */
	"[15]",	"|",	"[17]",	"s",	"T",		/* 15 - 19 */
	"*",	"D",	"\"",	"=",	"p",		/* 20 - 24 */
	"B",	"^",	"t",	"+",	"u",		/* 25 - 29 */
	"?",	"!",	"[",	"]",	"e",		/* 30 - 34 */
	"n",	"@",	"x",	"f",	"(",		/* 35 - 39 */
	")",	"r",    "[42]",	"[43]",	"[44]",		/* 40 - 44 */
	"[45]",	"[46]",	"[47]",	"[48]",	"[49]"		/* 45 - 49 */
};

FSTRING ecgstr(code)
int code;
{
    static char buf[9];

    if (0 <= code && code <= ACMAX)
	return (cstring[code]);
    else {
	(void)sprintf(buf,"[%d]", code);
	return (buf);
    }
}

FINT strecg(str)
char *str;
{
    int code;

    for (code = 1; code <= ACMAX; code++)
	if (strcmp(str, cstring[code]) == 0)
	    return (code);
    return (NOTQRS);
}

FINT setecgstr(code, string)
int code;
char *string;
{
    if (NOTQRS <= code && code <= ACMAX) {
	if (cstring[code] == NULL || strcmp(cstring[code], string)) {
	    char *p = malloc(strlen(string)+1);
	    if (p) strcpy(cstring[code] = p, string);
	}
	return (0);
    }
    wfdb_error("setecgstr: illegal annotation code %d\n", code);
    return (-1);
}

static char *astring[ACMAX+1] = {  /* mnemonic strings for each code */
        " ",	"N",	"L",	"R",	"a",		/* 0 - 4 */
	"V",	"F",	"J",	"A",	"S",		/* 5 - 9 */
	"E",	"j",	"/",	"Q",	"~",		/* 10 - 14 */
	"[15]",	"|",	"[17]",	"s",	"T",		/* 15 - 19 */
	"*",	"D",	"\"",	"=",	"p",		/* 20 - 24 */
	"B",	"^",	"t",	"+",	"u",		/* 25 - 29 */
	"?",	"!",	"[",	"]",	"e",		/* 30 - 34 */
	"n",	"@",	"x",	"f",	"(",		/* 35 - 39 */
	")",	"r",    "[42]",	"[43]",	"[44]",		/* 40 - 44 */
	"[45]",	"[46]",	"[47]",	"[48]",	"[49]"		/* 45 - 49 */
};

FSTRING annstr(code)
int code;
{
    static char buf[9];

    if (0 <= code && code <= ACMAX)
	return (astring[code]);
    else {
	(void)sprintf(buf,"[%d]", code);
	return (buf);
    }
}

FINT strann(str)
char *str;
{
    int code;

    for (code = 1; code <= ACMAX; code++)
	if (strcmp(str, astring[code]) == 0)
	    return (code);
    return (NOTQRS);
}

FINT setannstr(code, string)
int code;
char *string;
{
    if (0 < code && code <= ACMAX) {
	if (astring[code] == NULL || strcmp(astring[code], string)) {
	    char *p = malloc(strlen(string)+1);
	    if (p) {
		strcpy(astring[code] = p, string);
		modified[code] = 1;
	    }
	}
	return (0);
    }
    else if (-ACMAX < code && code <= 0) {
	if (astring[-code] == NULL || strcmp(astring[-code], string)) {
	    char *p = malloc(strlen(string)+1);
	    if (p) strcpy(astring[-code] = p, string);
	}
	return (0);
    }
    else {
	wfdb_error("setannstr: illegal annotation code %d\n", code);
	return (-1);
    }
}

static char *tstring[ACMAX+1] = {  /* descriptive strings for each code */
    "",
    "Normal beat",
    "Left bundle branch block beat",
    "Right bundle branch block beat",
    "Aberrated atrial premature beat",
    "Premature ventricular contraction",
    "Fusion of ventricular and normal beat",
    "Nodal (junctional) premature beat",
    "Atrial premature beat",
    "Supraventricular premature or ectopic beat",
    "Ventricular escape beat",
    "Nodal (junctional) escape beat",
    "Paced beat",
    "Unclassifiable beat",
    "Change in signal quality",
    (char *)NULL,
    "Isolated QRS-like artifact",
    (char *)NULL,
    "ST segment change",
    "T-wave change",
    "Systole",
    "Diastole",
    "Comment annotation",
    "Measurement annotation",
    "P-wave peak",
    "Bundle branch block beat (unspecified)",
    "(Non-captured) pacemaker artifact",
    "T-wave peak",
    "Rhythm change",
    "U-wave peak",
    "Beat not classified during learning",
    "Ventricular flutter wave",
    "Start of ventricular flutter/fibrillation",
    "End of ventricular flutter/fibrillation",
    "Atrial escape beat",
    "Supraventricular escape beat",
    "Link to external data (aux contains URL)",
    "Non-conducted P-wave (blocked APC)",
    "Fusion of paced and normal beat",
    "Waveform onset",
    "Waveform end",
    "R-on-T premature ventricular contraction",
    (char *)NULL,
    (char *)NULL,
    (char *)NULL,
    (char *)NULL,
    (char *)NULL,
    (char *)NULL,
    (char *)NULL,
    (char *)NULL
};

FSTRING anndesc(code)
int code;
{
    if (0 <= code && code <= ACMAX)
	return (tstring[code]);
    else
	return ("illegal annotation code");
}

FINT setanndesc(code, string)
int code;
char *string;
{
    if (0 < code && code <= ACMAX) {
	if (tstring[code] == NULL || strcmp(tstring[code], string)) {
	    char *p = malloc(strlen(string)+1);
	    if (p) {
		strcpy(tstring[code] = p, string);
		modified[code] = 1;
	    }
	}
	return (0);
    }
    else if (-ACMAX < code && code <= 0) {
	if (tstring[-code] == NULL || strcmp(tstring[-code], string)) {
	    char *p = malloc(strlen(string)+1);
	    if (p) strcpy(tstring[-code] = p, string);
	}
	return (0);
    }
    else {
	wfdb_error("setanndesc: illegal annotation code %d\n", code);
	return (-1);
    }
}

FVOID iannclose(n)      /* close input annotation file n */
WFDB_Annotator n;
{
    struct iadata *ia;

    if (n < niaf && (ia = iad[n]) != NULL && ia->file != NULL) {
	(void)wfdb_fclose(ia->file);
	(void)free(ia->info.name);
	(void)free(ia);
	while (n < niaf-1) {
	    iad[n] = iad[n+1];
	    n++;
	}
	iad[n] = NULL;
	niaf--;
	maxiann--;
    }
}

FVOID oannclose(n)      /* close output annotation file n */
WFDB_Annotator n;
{
    int i;
    char cmdbuf[256];
    struct oadata *oa;

    if (n < noaf && (oa = oad[n]) != NULL && oa->file != NULL) {
	switch (oa->info.stat) {
	  case WFDB_WRITE:	/* write logical EOF for MIT-format files */
	    wfdb_p16(0, oa->file);
	    break;
	  case WFDB_AHA_WRITE:	/* write logical EOF for AHA-format files */
	    i = ABLKSIZ - (unsigned)(wfdb_ftell(oa->file)) % ABLKSIZ;
	    while (i-- > 0)
		(void)wfdb_putc(EOAF, oa->file);
	    break;
	}
	(void)wfdb_fclose(oa->file);
	if (oa->out_of_order) {
	    int dosort = DEFWFDBANNSORT;
	    char *p = getenv("WFDBANNSORT");

	    if (p) dosort = atoi(p);
	    if (dosort) {
		if (system(NULL) != 0) {
		    wfdb_error(
			 "Rearranging annotations for output annotator %s ...",
			 oa->info.name);
		    (void)sprintf(cmdbuf, "sortann -r %s -a %s",
				  oa->rname, oa->info.name);
		    if (system(cmdbuf) == 0) {
			wfdb_error("done!");
			oa->out_of_order = 0;
		    }
		    else
		      wfdb_error(
			       "\nAnnotations still need to be rearranged.\n");
		}
	    }
	}
	if (oa->out_of_order) {
	    wfdb_error("Use the command:\n  sortann -r %s -a %s\n",
		       oa->rname, oa->info.name);
	    wfdb_error("to rearrange annotations in the correct order.\n");
	}
	(void)free(oa->info.name);
	(void)free(oa->rname);
	(void)free(oa);
	while (n < noaf-1) {
	    oad[n] = oad[n+1];
	    n++;
	}
	oad[n] = NULL;
	noaf--;
	maxoann--;
    }
}

/* Private functions (for the use of other WFDB library functions only). */

void wfdb_oaflush()
{
    unsigned int i;

    for (i = 0; i < noaf; i++)
	(void)wfdb_fflush(oad[i]->file);
}

void wfdb_anclose()
{
    WFDB_Annotator an;

    for (an = niaf; an != 0; an--)
	iannclose(an-1);
    for (an = noaf; an != 0; an--)
	oannclose(an-1);
}
