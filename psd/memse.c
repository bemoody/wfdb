/* file: memse.c	G. Moody	6 February 1992
			Last revised:    16 June 2003

-------------------------------------------------------------------------------
memse: Estimate power spectrum using maximum entropy (all poles) method
Copyright (C) 2003 George B. Moody

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
please visit PhysioNet (http://www.physionet.org/).
_______________________________________________________________________________

This program has been written to behave as much like `fft' as possible.  The
input and output formats and many of the options are the same.  See the man
page (memse.1) for details.

This version agrees with `fft' output (amplitude spectrum with total power
equal to the variance);  thanks to Joe Mietus.
*/

#include <stdio.h>
#include <math.h>
#ifndef __STDC__
extern double atof();
#else
#include <stdlib.h>
#endif

#define PI	 M_PI	/* pi to machine precision, defined in math.h */

#ifdef i386
#define strcasecmp strcmp
#endif

double wsum;

/* See Oppenheim & Schafer, Digital Signal Processing, p. 241 (1st ed.) */
double win_bartlett(j, n)
int j, n;
{
    double a = 2.0/(n-1), w;

    if ((w = j*a) > 1.0) w = 2.0 - w;
    wsum += w;
    return (w);
}

/* See Oppenheim & Schafer, Digital Signal Processing, p. 242 (1st ed.) */
double win_blackman(j, n)
int j, n;
{
    double a = 2.0*PI/(n-1), w;

    w = 0.42 - 0.5*cos(a*j) + 0.08*cos(2*a*j);
    wsum += w;
    return (w);
}

/* See Harris, F.J., "On the use of windows for harmonic analysis with the
   discrete Fourier transform", Proc. IEEE, Jan. 1978 */
double win_blackman_harris(j, n)
int j, n;
{
    double a = 2.0*PI/(n-1), w;

    w = 0.35875 - 0.48829*cos(a*j) + 0.14128*cos(2*a*j) - 0.01168*cos(3*a*j);
    wsum += w;
    return (w);
}

/* See Oppenheim & Schafer, Digital Signal Processing, p. 242 (1st ed.) */
double win_hamming(j, n)
int j, n;
{
    double a = 2.0*PI/(n-1), w;

    w = 0.54 - 0.46*cos(a*j);
    wsum += w;
    return (w);
}

/* See Oppenheim & Schafer, Digital Signal Processing, p. 242 (1st ed.)
   The second edition of Numerical Recipes calls this the "Hann" window. */
double win_hanning(j, n)
int j, n;
{
    double a = 2.0*PI/(n-1), w;

    w = 0.5 - 0.5*cos(a*j);
    wsum += w;
    return (w);
}

/* See Press, Flannery, Teukolsky, & Vetterling, Numerical Recipes in C,
   p. 442 (1st ed.) */
double win_parzen(j, n)
int j, n;
{
    double a = (n-1)/2.0, w;

    if ((w = (j-a)/(a+1)) > 0.0) w = 1 - w;
    else w = 1 + w;
    wsum += w;
    return (w);
}

/* See any of the above references. */
double win_square(j, n)
int j, n;
{
    wsum += 1.0;
    return (1.0);
}

/* See Press, Flannery, Teukolsky, & Vetterling, Numerical Recipes in C,
   p. 442 (1st ed.) or p. 554 (2nd ed.) */
double win_welch(j, n)
int j, n;
{
    double a = (n-1)/2.0, w;

    w = (j-a)/(a+1);
    w = 1 - w*w;
    wsum += w;
    return (w);
}

char *pname;

FILE *ifile;
float *data, *wk1, *wk2;
long nmax = 512L;	/* Initial buffer size (must be a power of 2).
			   Note that input() will increase this value as
			   necessary by repeated doubling, depending on
			   the length of the input series. */
float *wkm;
int fflag;
unsigned int len;
unsigned int nout;
unsigned int poles;
int wflag;
int zflag;
double freq, (*window)();

main(argc, argv)
int argc;
char *argv[];
{
    int i, pflag = 0;
    char *prog_name();
    double df;
    float pm, *cof, evlmem();
    FILE *fp;
    long input();
    void detrend(), error(), help(), memcof();

    pname = prog_name(argv[0]);
    for (i = 1; i < argc; i++) {
	if (*argv[i] == '-') switch (*(argv[i]+1)) {
	  case 'f':	/* print frequencies */
	    if (++i >= argc)
		error("sampling frequency must follow -f");
	    freq = atof(argv[i]);
	    fflag = 1;
	    break;
	  case 'h':	/* print help and exit */
	    help();
	    exit(0);
	    break;
	  case 'l':	/* handle up to n-point input series (obsolete) */
	    fprintf(stderr,
	    "%s: -l option is obsolete (%s can handle inputs of any length)\n",
		    pname, pname);
	    ++i;
	    break;
	  case 'n':	/* print n equally-spaced output values */
	    if (++i >= argc)
		error("output length must follow -n");
	    nout = atoi(argv[i]);
	    break;
	  case 'o':	/* specify the model order (number of poles) */
	    if (++i >= argc)
		error("order (number of poles) must follow -o");
	    poles = atoi(argv[i]);
	    break;
	  case 'P':     /* print power spectrum (squared magnitudes) */
	    pflag = 1;
	    break;
	  case 'w':	/* apply windowing function to input */
	    if (++i >= argc)
		error("window type must follow -w");
	    if (strcasecmp(argv[i], "Bartlett") == 0)
		window = win_bartlett;
	    else if (strcasecmp(argv[i], "Blackman") == 0)
		window = win_blackman;
	    else if (strcasecmp(argv[i], "Blackman-Harris") == 0)
		window = win_blackman_harris;
	    else if (strcasecmp(argv[i], "Hamming") == 0)
		window = win_hamming;
	    else if (strcasecmp(argv[i], "Hanning") == 0)
		window = win_hanning;
	    else if (strcasecmp(argv[i], "Parzen") == 0)
		window = win_parzen;
	    else if (strcasecmp(argv[i], "Square") == 0 ||
		     strcasecmp(argv[i], "Rectangular") == 0 ||
		     strcasecmp(argv[i], "Dirichlet") == 0)
		window = win_square;
	    else if (strcasecmp(argv[i], "Welch") == 0)
		window = win_welch;
	    else {
		fprintf(stderr, "%s: unrecognized window type %s\n",
			pname, argv[i]);
		exit(1);
	    }
	    wflag = 1;
	    break;
	  case 'z':			/* zero-mean the input */
	    zflag = 1;
	    break;
	  case 'Z':			/* zero-mean and detrend the input */
	    zflag = 2;
	    break;
	  case '\0':			/* read data from standard input */
	    ifile = stdin;
	    break;
	  default:
	    fprintf(stderr, "%s: unrecognized option %s ignored\n",
		    pname, argv[i]);
	    break;
	    }
	else if (i == argc-1) {	/* last argument: input file name */
	    if ((ifile = fopen(argv[i], "rt")) == NULL) {
		fprintf(stderr, "%s: can't open %s\n", pname, argv[i]);
		exit(2);
	    }
	}
    }
    if (ifile == NULL) {
	help();
	exit(1);
    }

    /* Read the input series. */
    len = input();

    /* Check the model order. */
    if (poles > len) poles = len;
    if ((double)poles*poles > len)
	fprintf(stderr,
		"%s: the model order (number of poles) may be too high\n",
		pname);

    /* Set the model order to a reasonable value if it is unspecified. */
    if (poles == 0) {
	poles = (int)(sqrt((double)len) + 0.5);
	fprintf(stderr,
		"%s: using a model order of %d\n", pname, poles);
    }

    /* Allocate arrays for coefficients. */
    if ((cof = (float *)malloc((unsigned)poles*sizeof(float))) == NULL ||
	(wkm = (float *)malloc((unsigned)poles*sizeof(float))) == NULL) {
	fprintf(stderr, "%s: insufficient memory\n", pname);
	exit(1);
    }

    /* Zero-mean, detrend, and/or window the input series as required. */
    if (zflag) {
	double rmean, rsum = 0;

	for (i = 0; i < len; i++)
	    rsum += data[i];
	rmean = rsum/len;
	for (i = 0; i < len; i++)
	    data[i] -= rmean;
	if (zflag == 2)
	    detrend(data, len);
    }
    if (wflag)
	for (i = 0; i < len; i++)
	    data[i] *= (*window)(i, len);
    
    /* Calculate coefficients for MEM spectral estimation. */
    memcof(data, len, poles, &pm, cof);

    /* If the number of output points was not specified, choose the largest
       power of 2 less than len, plus 1 (so that the number of output points
       matches that produced by an FFT). */
    if (nout == 0) {
	while (nmax >= len)
	    nmax /= 2;;
	nout = nmax + 1;
    }

    /* Print outputs. */
    for (i = 0, df = 0.5/(nout-1); i < nout; i++) {
	if (fflag) printf("%g\t", i*df*freq);
	if (pflag)
	    printf("%g\n", evlmem(i*df, cof, poles, pm)/(nout-1));
	else
	    printf("%g\n", sqrt(evlmem(i*df, cof, poles, pm)/(nout-1)));
    }

    free(cof);
    free(data);
    free(wk1);
    free(wk2);
    free(wkm);

    exit(0);
}

/* Calculate coefficients for MEM spectral estimation.  See Numerical Recipes,
   pp. 447-451. */
void memcof(data, n, m, pm, cof)
float *data, *pm, *cof;
unsigned int n, m;
{
    int i, j, k;
    float denom, num, p = 0.0;

    for (j = 0; j < n; j++)
	p += data[j]*data[j];
    *pm = p/n;
    wk1[0] = data[0];
    wk2[n-2] = data[n-1];
    for (j = 1; j < n-1; j++) {
	wk1[j] = data[j];
	wk2[j-1] = data[j];
    }
    for (k = 0; k < m; k++) {
	for (j = 0, num = denom = 0.0; j < n-k-1; j++) {
	    num += wk1[j]*wk2[j];
	    denom += wk1[j]*wk1[j] + wk2[j]*wk2[j];
	}
	cof[k] = 2.0*num/denom;
	*pm *= 1.0 - cof[k]*cof[k];
	if (k)
	    for (i = 0; i < k; i++)
		cof[i] = wkm[i] - cof[k]*wkm[k-i-1];
	if (k != m-1) {
	    for (i = 0; i <= k; i++)
		wkm[i] = cof[i];
	    for (j = 0; j < n-k-2; j++) {
		wk1[j] -= wkm[k]*wk2[j];
		wk2[j] = wk2[j+1] - wkm[k]*wk1[j+1];
	    }
	}
    }
}

/* Evaluate power spectral estimate at f (0 <= f < = 0.5, where 1 is the
   sampling frequency), given MEM coefficients in cof[0 ... m-1] and pm
   (see Numerical Recipes, pp. 451-452). */
float evlmem(f, cof, m, pm)
double f;
float *cof, pm;
unsigned int m;
{
    int i;
    float sumr = 1.0, sumi = 0.0;
    double wr = 1.0, wi = 0.0, wpr, wpi, wt, theta;

    theta = 2.0*PI*f;
    wpr = cos(theta);
    wpi = sin(theta);
    for (i = 0; i < m; i++) {
	wt = wr;
	sumr -= cof[i]*(wr = wr*wpr - wi*wpi);
	sumi -= cof[i]*(wi = wi*wpr + wt*wpi);
    }
    return (pm/(sumr*sumr+sumi*sumi));
}


/* This function detrends (subtracts a least-squares fitted line from) a
   a sequence of n uniformily spaced ordinates supplied in c. */
void detrend(c, n)
float *c;
int n;
{
    int i;
    double a, b = 0.0, tsqsum = 0.0, ysum = 0.0, t;

    for (i = 0; i < n; i++)
	ysum += c[i];
    for (i = 0; i < n; i++) {
	t = i - n/2 + 0.5;
	tsqsum += t*t;
	b += t*c[i];
    }
    b /= tsqsum;
    a = ysum/n - b*(n-1)/2.0;
    for (i = 0; i < n; i++)
	c[i] -= a + b*i;
    if (b < -0.04 || b > 0.04)
	fprintf(stderr,
		"%s: (warning) possibly significant trend in input series\n",
		pname);
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

void error(s)
char *s;
{
    fprintf(stderr, "%s: %s\n", pname, s);
    exit(1);
}

static char *help_strings[] = {
"usage: %s [ OPTIONS ...] INPUT-FILE\n",
" where INPUT-FILE is the name of a text file containing a time series",
" (use `-' to read the standard input), and OPTIONS may be any of:",
" -f FREQ  Show the center frequency for each bin in the first column.  The",
"          FREQ argument specifies the input sampling frequency;  the center",
"          frequencies are given in the same units.",
" -h       Print on-line help.",
" -n N     Print N equally-spaced output values; default: N = half the number",
"          of input samples.",
" -o P     Specify the model order (number of poles); default: P = the square",
"          root of the number of input samples.",
" -P       Generate a power spectrum (print squared magnitudes).",
" -w WINDOW",
"          Apply the specified WINDOW to the input data.  WINDOW may be one",
"          of: `Bartlett', `Blackman', `Blackman-Harris', `Hamming',",
"          `Hanning', `Parzen', `Square', and `Welch'.",
" -z       Zero-mean the input data.",
" -Z       Detrend and zero-mean the input data.",
NULL
};

void help()
{
    int i;

    (void)fprintf(stderr, help_strings[0], pname);
    for (i = 1; help_strings[i] != NULL; i++)
	(void)fprintf(stderr, "%s\n", help_strings[i]);
}

/* Read input data, allocating and filling x[] and y[].  The return value is
   the number of points read.

   This function allows the input buffers to grow as large as necessary, up to
   the available memory (assuming that a long int is large enough to address
   any memory location). */

long input()
{
    unsigned long npts = 0L;

    if ((data = (float *)malloc(nmax * sizeof(float))) == NULL ||
	(wk1 = (float *)malloc(64 * nmax * sizeof(float))) == NULL ||
	(wk2 = (float *)malloc(64 * nmax * sizeof(float))) == NULL) {
	if (data) (void)free(data);
	if (wk1) (void)free(wk1);
	fclose(ifile);
	error("insufficient memory");
    }

    while (fscanf(ifile, "%f", &data[npts]) == 1) {
        if (++npts >= nmax) {	/* double the size of the input buffers */
	    unsigned long nmaxt = nmax << 1;
	    float *datat, *w1t, *w2t;

	    if (nmaxt * sizeof(float) < nmax) {
		fprintf(stderr,
		      "%s: insufficient memory, truncating input at row %d\n",
		      pname, npts);
	        break;
	    }
	    if ((datat = (float *)realloc(data,nmaxt*sizeof(float))) == NULL) {
		fprintf(stderr,
		      "%s: insufficient memory, truncating input at row %d\n",
		      pname, npts);
	        break;
	    }
	    data = datat;
	    if ((w1t = (float *)realloc(wk1,64*nmaxt*sizeof(float))) == NULL){
		fprintf(stderr,
		      "%s: insufficient memory, truncating input at row %d\n",
		      pname, npts);
	        break;
	    }
	    wk1 = w1t;
	    if ((w2t = (float *)realloc(wk2,64*nmaxt*sizeof(float))) == NULL){
		fprintf(stderr,
		      "%s: insufficient memory, truncating input at row %d\n",
		      pname, npts);
	        break;
	    }
	    wk2 = w2t;
	    nmax = nmaxt;
	}
    }

    fclose(ifile);
    if (npts < 1) error("no data read");
    return (npts);
}
