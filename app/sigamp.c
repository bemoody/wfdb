/* file: sigamp.c	G. Moody	30 November 1991
			Last revised:	   4 May 1999

-------------------------------------------------------------------------------
sigamp: Measure signal amplitudes
Copyright (C) 1999 George B. Moody

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA.

You may contact the author by e-mail (george@mit.edu) or postal mail
(MIT Room E25-505A, Cambridge, MA 02139 USA).  For updates to this software,
please visit the author's web site (http://ecg.mit.edu/).
_______________________________________________________________________________

*/


#include <stdio.h>
#include <math.h>
#ifndef __STDC__
extern void exit();
#endif

#include <wfdb/wfdb.h>
#define isqrs
#define map2
#define ammap
#define mamap
#define annpos
#include <wfdb/ecgmap.h>

#define NMAX 300

char *pname;
int namp;
int nsig;
int pflag;		/* if non-zero, print physical units */
int vflag;		/* if non-zero, print individual measurements */
double amp[WFDB_MAXSIG][NMAX];
long dt1, dt2, dtw;

main(argc, argv)
int argc;
char *argv[];
{
    char *record = NULL, *prog_name();
    int i, j, jlow, jhigh, ampcmp(), getptp(), getrms();
    long from = 0L, to = 0L, t;
    static WFDB_Siginfo si[WFDB_MAXSIG];
    static WFDB_Anninfo ai;
    void help();

    pname = prog_name(argv[0]);
    for (i = 1; i < argc; i++) {
	if (*argv[i] == '-') switch(*(argv[i]+1)) {
	  case 'a':
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: annotator name must follow -a\n",
			      pname);
		exit(1);
	    }
	    ai.name = argv[i];
	    break;
	  case 'd':
	    if (++i >= argc-1) {
		(void)fprintf(stderr, "%s: DT1 and DT2 must follow -d\n",
			      pname);
		exit(1);
	    }
	    /* save arg list index, convert arguments to samples later (after
	       record has been opened and sampling frequency is known) */
	    dt1 = i++;
	    break;
	  case 'f':
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: start time must follow -f\n",
			      pname);
		exit(1);
	    }
	    /* save arg list index, convert argument to samples later */
	    from = i;
	    break;
	  case 'h':	/* help requested */
	    help();
	    exit(0);
	    break;
	  case 'p':
	    pflag = 1;
	    break;
	  case 'r':
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: record name must follow -r\n",
			      pname);
		exit(1);
	    }
	    record = argv[i];
	    break;
	  case 't':
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: stop time must follow -t\n",
			      pname);
		exit(1);
	    }
	    /* save arg list index, convert argument to samples later */
	    to = i;
	    break;
	  case 'v':
	    vflag = 1;
	    break;
	  case 'w':
	    if (++i >= argc) {
		(void)fprintf(stderr, "%s: window size must follow -w\n",
			      pname);
		exit(1);
	    }
	    /* save arg list index, convert argument to samples later */
	    dtw = i;
	    break;
	  default:
	    (void)fprintf(stderr, "%s: unrecognized option %s\n",
			  pname, argv[i]);
	    exit(1);
	    break;
	}
	else {
	    (void)fprintf(stderr, "%s: unrecognized argument %s\n",
			  pname, argv[i]);
	    exit(1);
	    break;
	}
    }
    if (record == NULL) {
	help();
	exit(1);
    }
    if ((nsig = isigopen(record, si, WFDB_MAXSIG)) <= 0) exit(2);
    for (i = 0; i < nsig; i++)
	if (si[i].gain == 0.0) si[i].gain = WFDB_DEFGAIN;
    if (from > 0L) from = strtim(argv[(int)from]);
    if (to > 0L) to = strtim(argv[(int)to]);
    if (from < 0L) from = -from;
    if (to < 0L) to = -to;
    if (from >= to) to = strtim("e");
    if (ai.name) {
	WFDB_Annotation annot;

	if (dtw > 0L) {
	    (void)fprintf(stderr,
			  "%s: -a and -w options cannot be used together;\n",
			  pname);
	    (void)fprintf(stderr, "  -w option is being ignored\n");
	}
	if (annopen(record, &ai, 1) < 0) exit(2);
	if (dt1 == 0L) dt1 = -(dt2 = strtim("0.05"));
	else {
	    char *p = argv[(int)dt1+1];

	    if (*p == '-') dt2 = -strtim(p+1);
	    else dt2 = strtim(p);
	    p = argv[(int)dt1];
	    if (*p == '-') dt1 = strtim(p+1);
	    else dt1 = -strtim(p);
	    if (dt1 > dt2) {
		long temp = dt1;
		
		dt1 = dt2; dt2 = temp;
	    }
	}
	if (dt1 == dt2) dt2++;
	if (iannsettime(from) < 0) exit(2);
	while (getann(0, &annot) == 0 && namp < NMAX &&
	       (to == 0L || annot.time < to)) {
	    if (map1(annot.anntyp) == NORMAL) {
		if (getptp(annot.time) < 0) break;
		if (vflag) {
		    (void)printf("%s", mstimstr(annot.time));
		    if (!pflag)
			for (i = 0; i < nsig; i++)
			    (void)printf("\t%g", amp[i][namp]);
		    else
			for (i = 0; i < nsig; i++)
			    (void)printf("\t%g", amp[i][namp]/si[i].gain);
		    (void)printf("\n");
		}
		namp++;
	    }
	}
    }
    else {
	if (dt1 > 0L) {
	    (void)fprintf(stderr,
			  "%s: -d option must be used with -a;\n", pname);
	    (void)fprintf(stderr, "  -d option is being ignored\n");
	}
	if (dtw == 0L || (dtw = strtim(argv[(int)dtw])) <= 0L)
	    dtw = strtim("1");
	if (from > 0L && isigsettime(from) < 0L) exit(2);
	for (t = from; namp < NMAX && (to == 0L || t < to); namp++, t += dtw) {
	    if (getrms(t) < 0) break;
	    if (vflag) {
		(void)printf("%s", mstimstr(t));
		if (!pflag)
		    for (i = 0; i < nsig; i++)
			(void)printf("\t%g", amp[i][namp]);
		else
		    for (i = 0; i < nsig; i++)
			(void)printf("\t%g", amp[i][namp]/si[i].gain);
		(void)printf("\n");
	    }
	}
    }
    jlow = namp/20;
    jhigh = namp - jlow;
    if (vflag)
	(void)printf("Trimmed mean");
    for (i = 0; i < nsig; i++) {
	double a;

	qsort((char*)amp[i], namp, sizeof(double), ampcmp);
	for (a = 0.0, j = jlow; j < jhigh; j++)
	    a += amp[i][j];
	a /= jhigh - jlow;
	if (!pflag)
	    (void)printf("\t%g", a);
	else
	    (void)printf("\t%g", a/si[i].gain);
    }
    (void)printf("\n");
    exit(0); /*NOTREACHED*/
}

int getrms(t)
long t;
{
    int i, v, vv[WFDB_MAXSIG], v0[WFDB_MAXSIG];
    long tt;
    double vmean[WFDB_MAXSIG], vsum[WFDB_MAXSIG];

    if (getvec(vv) < nsig) return (-1);

    for (i = 0; i < nsig; i++) {
	vmean[i] = v0[i] = vv[i];
	vsum[i] = 0.0;
    }

    for (tt = 1; tt < dtw; tt++) {
	if (getvec(vv) < nsig) return (-1);
	for (i = 0; i < nsig; i++)
	    vmean[i] += vv[i];
    }

    if (isigsettime(t) < 0 || getvec(vv) < nsig) return (-1);

    /* The quantity vv[i] - v0[i] is normally zero, but may be non-zero if
       the signals are stored in difference format (since isigsettime may
       introduce a DC error in this case).  In the calculation of vmean[i]
       below, this quantity is used to correct any such error. */
    for (i = 0; i < nsig; i++) {
	vmean[i] = vmean[i]/dtw + vv[i] - v0[i];
	v = vv[i] - vmean[i];
	vsum[i] += (double)v*v;
    }

    for (tt = 1; tt < dtw; tt++) {
	if (getvec(vv) < nsig) return (-1);
	for (i = 0; i < nsig; i++) {
	    v = vv[i] - vmean[i];
	    vsum[i] += (double)v*v;
	}
    }

    for (i = 0; i < nsig; i++)
	amp[i][namp] = sqrt(vsum[i]/dtw);

    return (0);
}

int getptp(t)
long t;
{
    int i, vmax[WFDB_MAXSIG], vmin[WFDB_MAXSIG], vv[WFDB_MAXSIG];
    long tt;

    if (isigsettime(t + dt1) < 0) return (-1);

    if (getvec(vmin) < nsig) return (-1);

    for (i = 0; i < nsig; i++)
	vmax[i] = vmin[i];

    for (tt = dt1; tt < dt2; tt++) {
	if (getvec(vv) < nsig) return (-1);
	for (i = 0; i < nsig; i++) {
	    if (vv[i] < vmin[i]) vmin[i] = vv[i];
	    else if (vv[i] > vmax[i]) vmax[i] = vv[i];
	}
    }

    for (i = 0; i < nsig; i++)
	amp[i][namp] = vmax[i] - vmin[i];

    return (0);
}

int ampcmp(p1, p2)
double *p1, *p2;
{
    if (*p1 > *p2) return (1);
    else if (*p1 == *p2) return (0);
    else return (-1);
}

char *prog_name(s)
char *s;
{
    char *p = s + strlen(s);

#ifdef MSDOS
    while (p >= s && *p != '\\' && *p != ':') {
	if (*p == '.')
	    *p = '\0';		/* strip off extension */
	if ('A' <= *p && *p <= 'Z')
	    *p += 'a' - 'A';	/* convert to lower case */
	p--;
    }
#else
    while (p >= s && *p != '/')
	p--;
#endif
    return (p+1);
}

static char *help_strings[] = {
 "usage: %s -r RECORD [OPTIONS ...]\n",
 "where OPTIONS may include any of:",
 " -a ANN      specify annotator, measure amplitudes near QRS annotations",
 " -d DT1 DT2  set measurement window relative to QRS annotations",
 "              defaults: DT1 = 0.05 (seconds before annotation);",
 "              DT2 = 0.05 (seconds after annotation)",
 " -f TIME     begin at specified time",
 " -h          print this usage summary",
 " -p          print results in physical units (default: ADC units)",
 " -t TIME     stop at specified time",
 " -v          verbose mode: print individual measurements",
 " -w DTW      set RMS amplitude measurement window",
 "              default: DTW = 1 (second)",
NULL
};

void help()
{
    int i;

    (void)fprintf(stderr, help_strings[0], pname);
    for (i = 1; help_strings[i] != NULL; i++)
	(void)fprintf(stderr, "%s\n", help_strings[i]);
}
