/* file: view.c         G. Moody         July 1989
			Last revised:   4 May 1999

-------------------------------------------------------------------------------
view: DB browser for MS-DOS (using Microsoft C graphics library)
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
please visit PhysioNet (http://www.physionet.org/).
_______________________________________________________________________________

*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <graph.h>
#include <dos.h>
#include <wfdb/wfdb.h>
#include <wfdb/ecgcodes.h>

#define VERSION 5.1

/* This function returns 1 if the mouse and driver are installed, 0 if the
   driver is installed but not the mouse, and -1 otherwise.  See Microsoft
   Mouse Programmer's Reference (2nd ed.), p. 105. */
int findmouse()
{
	union REGS iReg, oReg;
	void (interrupt far *int_handler)();
	long vector;
	unsigned char first_byte;
	
	/* Determine the mouse driver interrupt address. */
	int_handler = _dos_getvect(0x33);

	/* Get the interrupt vector and the first instruction of the interrupt
	   service routine.  The vector should not be 0, and the first
	   instruction should not be `iret' (0xcf). */
	if ((vector = (long)int_handler) == 0L ||
		(first_byte = *(unsigned char far *)int_handler) == 0xcf)
			return (-1);    /* driver not loaded */

	/* Reset the mouse driver and get the mouse status. */
	iReg.x.ax = 0;
	int86(0x33, &iReg, &oReg);

	/* Was the mouse found? */
	if (oReg.x.ax == -1)
		return (1);     /* yes */
	else
		return (0);     /* no */
}

showmousecursor()
{
	union REGS iReg, oReg;

	iReg.x.ax = 1;
	int86(0x33, &iReg, &oReg);
}

hidemousecursor()
{
	union REGS iReg, oReg;

	iReg.x.ax = 2;
	int86(0x33, &iReg, &oReg);
}

struct mousestate {
	int x, y, b;
} ms;
int mpp = 1;    /* mouse pixels per display pixel */

getmousestate(struct mousestate *msp)
{
	union REGS iReg, oReg;

	iReg.x.ax = 3;
	int86(0x33, &iReg, &oReg);
	msp->x = oReg.x.cx/mpp;
	msp->y = oReg.x.dx;
	msp->b = oReg.x.bx;     /* 1: left button down, 2: right button down, 4:
						 center button down */
}

setmousecursor(x, y)
int x, y;
{
	union REGS iReg, oReg;

	iReg.x.ax = 4;
	iReg.x.cx = x*mpp;
	iReg.x.dx = y;
	int86(0x33, &iReg, &oReg);
}

unsigned getclicks(button)
int button;     /* 0: left, 1: right, 2: center (?) */
{
	union REGS iReg, oReg;

	iReg.x.ax = 5;
	iReg.x.bx = button;
	int86(0x33, &iReg, &oReg);
	return ((unsigned)oReg.x.bx);
}

int left, top, right, bottom, use_gtext, use_mouse, xs, ys, yb, xppcol,
	ypprow;
double x, y;
double sps;
double xppmm, yppmm;
double xppsi, yppadu[MAXSIG], ysc;

#define xtr(A)  ((int)((A)*xppmm + left))
#define ytr(A)  ((int)(bottom - (A)*yppmm))
#define xt(A)   ((int)((A)*xppsi + left))
#define yt(A)   ((int)(yb - (A)*ysc))
#define tx(A)   (xt(A)/xppcol + 1)
#define ty(A)   (yt(A)/ypprow + 1)

#define move(T,V)       (_moveto(xt(T), yt(V)))
#define cont(T,V)       (_lineto(xt(T), yt(V)))
#define contix(I, V)    (_lineto((I)+left, yt(V)))

#if _MSC_VER < 600
#define _outgtext(A) _outtext(A)
#endif

#if _MSC_VER < 700
#define _getch getch
#define _rccoord rccoord
#endif

void line(double ax, double ay, double bx, double by)
{
    _moveto(xtr(ax), ytr(ay));
    _lineto(xtr(bx), ytr(by));
}

char *ltimstr(t)
long t;
{
	char *p = timstr(t);
	
	while (*p == ' ')
		p++;
	return (p);
}

int mflag = 1;          /* show annotation marker bars if non-zero */
long background_color;  /* _setbkcolor needs long, _setcolor needs int */
int annotation_color, button_text_color, button_color, cursor_color,
	grid_color, signal_color;
int grid_linestyle, marker_linestyle, signal_linestyle;

void grid()
{
    double yl, yh;

    _setlinestyle(grid_linestyle);
    _setcolor(grid_color);
    for (x = 0.; x <= xs; x += 5.)
	line(x, 0., x, (double)ys);
    for (x -= 5., yl = yh = ys/2.; yl >= 0.; yl -= 5., yh += 5.) {
	line(0., yl, x, yl);
	if (yl != yh) line(0., yh, x, yh);
    }
    if (mflag) _setlinestyle(marker_linestyle);
    else _setlinestyle(signal_linestyle);
}

char record[10], frametitle[80];
int nsig, trlen, *v, vmax[MAXSIG], vmin[MAXSIG];
int base[MAXSIG], v0, gflag = 0, prow, tby, tbmid, yppcm;
long tstart = 0L;
WFDB_Annotation annot;
WFDB_Siginfo si[MAXSIG];
struct videoconfig vc;
struct view_button {
	int left, right;
	char *label;
} back2, back1, forw1, forw2;
int tx0;

void draw_button(vb)
struct view_button vb;
{
	_settextcolor(button_text_color);
	_settextposition(prow,
		(vb.left+vb.right)/(2*xppcol) + (3-strlen(vb.label))/2);
	_outtext(vb.label);
	_setcolor(button_text_color);
	_arc(vb.left, bottom-ypprow+1, vb.left+2*xppcol, bottom-1,
		vb.left+xppcol, bottom-ypprow+1, vb.left+xppcol, bottom-1);
	_moveto(vb.left+xppcol, bottom-1);
	_lineto(vb.right-xppcol, bottom-1);
	_arc(vb.right-2*xppcol, bottom-ypprow+1, vb.right, bottom-1,
		vb.right-xppcol, bottom-1, vb.right-xppcol, bottom-ypprow+1);
	_moveto(vb.right-xppcol, bottom-ypprow+1);
	_lineto(vb.left+xppcol, bottom-ypprow+1);
	_setcolor(button_color);
	if (button_text_color)
		_floodfill(vb.left+xppcol, bottom-ypprow/2, button_text_color);
}

int compress = 0;

void do_disp()
{
    char *p, str[5];
    int i, j, s, *vp;

    if (use_mouse) hidemousecursor();
    _clearscreen(_GCLEARSCREEN);
    _displaycursor(_GCURSOROFF);
    if (use_mouse) {
	    _setcliprgn(left, top, right, bottom);
	    draw_button(back2);
	    draw_button(back1);
	    draw_button(forw1);
	    draw_button(forw2);
    }
    _settextcolor(grid_color);
    _settextposition(1, tx0);
    _outtext(frametitle);
    _setcliprgn(left, top+ypprow, right, bottom-ypprow);
    if (gflag) grid();
    else if (mflag) _setlinestyle(marker_linestyle);
    if (use_gtext) {
	_setcolor(signal_color);
	_moveto(left, bottom - 3*ypprow);
	_outgtext(ltimstr(tstart));
	_moveto(right - 8*xppcol, bottom - 3*ypprow);
	_outgtext(timstr(tstart+trlen));
    }
    else {
	_settextcolor(signal_color);
	_settextposition(prow-1, 1);
	_outtext(ltimstr(tstart));
	_settextposition(prow-1, vc.numtextcols - 8);
	_outtext(timstr(tstart+trlen));
	_settextcolor(annotation_color);
    }
    _setcolor(annotation_color);
    while (annot.time < tstart && getann(0, &annot) == 0)
	;
    while (annot.time < tstart + trlen) {
	switch (annot.anntyp) {
	  case RHYTHM:
	    tby = tbmid+1; p = annot.aux+1; break;
	  case STCH:
	  case TCH:
	  case NOTE:
	    tby = tbmid-1;
	    if (annot.aux) p = annot.aux+1;
	    else p = annstr(annot.anntyp); break;
	  case LINK:
	    tby = tbmid-1;
	    if (annot.aux) {
		char *p1 = annot.aux+1, *p2 = p1 + *(p1-1);
		p = p1;
		while (p1 < p2) {
		    if (*p1 == ' ' || *p1 == '\t') {
			p = p1 + 1;
			break;
		    }
		    p1++;
		}
		_setcolor(signal_color);
		_settextcolor(signal_color);
	    }
	    break;
	  case NOISE:
	    tby = tbmid-1;
	    if (annot.subtyp == -1) { p = "U"; break; }
	    /* The existing scheme is good for up to 4 signals;  it can be
	       easily extended to 8 or 12 signals using the chan and num
	       fields, or to an arbitrary number of signals using `aux'. */
	    for (s = 0; s < nsig && s < 4; s++) {
		if (annot.subtyp & (0x10 << s))
		    str[s] = 'u';       /* signal s is unreadable */
		else if (annot.subtyp & (0x01 << s))
		    str[s] = 'n';       /* signal s is noisy */
		else
		    str[s] = 'c';       /* signal s is clean */
	    }
	    str[s] = '\0';
	    p = str; break;
	  default:
	    tby = tbmid; p = annstr(annot.anntyp); break;
	}
	if (use_gtext) {
	    _moveto((int)((annot.time-tstart)*xppsi + left), tby*ypprow);
	    _outgtext(p);
	}
	else {
	    _settextposition(tby, tx(annot.time-tstart));
	    _outtext(p);
	}
	if (annot.anntyp == LINK) {
	    _setcolor(annotation_color);
	    _settextcolor(annotation_color);
	}
	if (mflag) {
	    int xx = (int)((annot.time-tstart)*xppsi) + left;

	    _moveto(xx, top);
	    _lineto(xx, (tby-1)*ypprow);
	    _moveto(xx, (tby+1)*ypprow);
	    _lineto(xx, bottom);
	}
	if (getann(0, &annot) != 0) break;
    }
    if (mflag) _setlinestyle(signal_linestyle);
    _setcolor(signal_color);
    if (!use_gtext)
	    _settextcolor(signal_color);
/*
    for (i = 0, vp = v; i < trlen; i++, vp += nsig) {
	if (getvec(vp) < nsig) break;
	for (s = 0; s < nsig; s++) {
	    if (vp[s] < vmin[s]) vmin[s] = vp[s];
	    else if (vp[s] > vmax[s]) vmax[s] = vp[s];
	}
    }
*/
    /* If there at least two samples per displayed column, compress the
       data. */
    if (compress) {
	static int x0, vv0[MAXSIG], vmn[MAXSIG], vmx[MAXSIG], vv[MAXSIG];
	getvec(vp = v);
	for (s = 0; s < nsig; s++)
	    vmax[s] = vmin[s] = vmn[s] = vmx[s] = vv0[s] = vp[s];
	for (i = 1, x0 = 0; i < trlen && getvec(vv) > 0; i++) {
		int xx;
	    if ((xx = i*xppsi) > x0) {
		x0 = xx;
		vp += nsig;
		for (s = 0; s < nsig; s++) {
		    if (vv[s] > vmx[s]) vmx[s] = vv[s];
		    else if (vv[s] < vmn[s]) vmn[s] = vv[s];
		    if (vmx[s] - vv0[s] > vv0[s] - vmn[s])
			vv0[s] = vmn[s] = vmx[s];
		    else
			vv0[s] = vmx[s] = vmn[s];
		    if ((vp[s] = vv0[s]) > vmax[s]) vmax[s] = vp[s];
		    else if (vp[s] < vmin[s]) vmin[s] = vp[s];
		}
	    }
	    else {
		for (s = 0; s < nsig; s++) {
		    if (vv[s] > vmx[s]) vmx[s] = vv[s];
		    else if (vv[s] < vmn[s]) vmn[s] = vv[s];
		}
	    }
	}
	i = (vp - v)/nsig;
    }
    else {
	for (i = 0, vp = v; i < trlen; i++, vp += nsig) {
	    if (getvec(vp) < nsig) break;
	    for (s = 0; s < nsig; s++) {
		if (vp[s] < vmin[s]) vmin[s] = vp[s];
		else if (vp[s] > vmax[s]) vmax[s] = vp[s];
	    }
	}
    }
    for (s = 0; s < nsig; s++) {
	int row;
	yb = base[s];
	v0 = (vmax[s] + vmin[s])/2;
	ysc = yppadu[s];
	if ((row = (yb - yppcm)/ypprow) < 1) row = 1;
	if (use_gtext) {
	    _moveto(left, row*ypprow);
	    _outgtext(si[s].desc);
	}
	else {
	    _settextposition(row, 1);
	    _outtext(si[s].desc);
	}
	vp = v+s;
	move(0, (*vp - v0));
	vp += nsig;
	if (compress)
	    for (j = 1; j < i; j++, vp += nsig)
		contix(j, (*vp - v0));
	else
	    for (j = 1; j < i; j++, vp += nsig)
		cont(j, (*vp - v0));
	vmax[s] = -9999; vmin[s] = 9999;
    }
    if (use_mouse) {
	    _setcolor(cursor_color);
	    showmousecursor();
    }
    _settextcolor(cursor_color);
    _settextposition(prow, 1);
    _outtext("> ");
    _displaycursor(_GCURSORON);
}

long save_background_color = -1L;
short save_text_color = -1;

void catch_signal(int sig)
{
   if (use_mouse)
	hidemousecursor();
   if (save_background_color >= 0L)
	   _setbkcolor(save_background_color);
   if (save_text_color >= 0)
	   _settextcolor(save_text_color);
   if (vc.numcolors >= 16)
	   _remappalette(15, _BRIGHTWHITE);
   _setvideomode(_DEFAULTMODE);
}

void restore_mode(void)
{
	catch_signal(0);
}

char command[21];
int asrch = 0, nann = 0;

void do_command(i)
int i;
{
    _settextposition(prow, 1);
    _outtext("> ");
    _outtext(command);
    switch (command[0]) {
	case 'x':
	case 'X':
		wfdbquit();
		restore_mode();
		exit(0);
	case 'g':
		gflag = 1-gflag;
		break;
	case 'm':
		mflag = 1-mflag;
		break;
	case '\0':      /* repeat previous search, or advance one screenful */
		if (asrch) strcpy(command, annstr(asrch));
		else strcpy(command, timstr(tstart + trlen));
				/* drop through to default case */
	default:                /* annotation mnemonic or time string */
		if (nann > 0 && (asrch = strann(command)) != NOTQRS) {
			while (getann(0, &annot) == 0 && annot.anntyp != asrch)
				;
			if (annot.anntyp == asrch)
				tstart = strtim(timstr(annot.time - trlen/2));
		}
		else {
			tstart = strtim(command);
			asrch = NOTQRS;
		}
		break;
	}
	if (isigsettime(tstart) < 0) {
		_settextposition(prow, 1);
		_outtext(">                      ");
		_settextposition(prow, 3);
		return;
	}
	if (nann > 0) {
		if (tstart == 0L) iannsettime(1L);
		else iannsettime(tstart);
		getann(0, &annot);
	}
	do_disp();
}

main(argc, argv)
int argc;
char *argv[];
{
    char *viewf, *viewp, *p;
    static char *cfname;
    static char *evp = "The value of VIEWP, `%s', appears garbled.\n%s";
    static char *evs = "Find and run `vsetup' before continuing.\n";
    int c, i, j, s;
    int vmode;
    static WFDB_Anninfo an;
    static WFDB_Calinfo ci;
    void help();
    
#ifdef MITCDROM
    char *wfdbp = getwfdb();
    if (*wfdbp == '\0')
	setwfdb(";\\mitdb;\\nstdb;\\stdb;\\vfdb;\\afdb;\\cdb;\\svdb;\\ltdb");
#endif
#ifdef STTCDROM
    char *wfdbp = getwfdb();
    if (*wfdbp == '\0')
	setwfdb(";\\edb;\\valedb");
#endif
#ifdef SLPCDROM
    char *wfdbp = getwfdb();
    if (*wfdbp == '\0')
	setwfdb(";\\slpdb");
#endif
#ifdef MGHCDROM
    char *wfdbp = getwfdb();
    if (*wfdbp == '\0')
	setwfdb(";\\mghdb");
#endif
    if ((viewp = getenv("VIEWP")) == NULL) {
	execlp("vsetup.exe", "vsetup", NULL);
	printf(evs);
	exit(1);
    }
    if ((p = strtok(viewp, ",")) == NULL) {
	printf(evp, viewp, evs);
	exit(1);
    }
    vmode = atoi(p);
    if ((p = strtok(NULL, ",")) == NULL) {
	printf(evp, viewp, evs);
	exit(1);
    }
    left = atoi(p);
    if (left < 0 || (p = strtok(NULL, ",")) == NULL) {
	printf(evp, viewp, evs);
	exit(1);
    }
    right = atoi(p);
    if (right <= left || (p = strtok(NULL, ",")) == NULL) {
	printf(evp, viewp, evs);
	exit(1);
    }
    top = atoi(p);
    if (top < 0 || (p = strtok(NULL, ",")) == NULL) {
	printf(evp, viewp, evs);
	exit(1);
    }
    bottom = atoi(p);
    if (bottom <= top || (p = strtok(NULL, ",")) == NULL) {
	printf(evp, viewp, evs);
	exit(1);
    }
    xs = atoi(p);
    p += strlen(p)+1;
    if (xs <= 0 || *p == '\0' || (ys = atoi(p)) <= 0) {
	printf(evp, viewp, evs);
	exit(1);
    }

    if (argc < 2) {
	printf("Enter the name of the record to view (or `?' for help): ");
	fgets(record, 10, stdin);
	record[strlen(record)-1] = '\0';
	if (strcmp(record, "?") == 0) {
	    help();
	    exit(1);
	}
    }
    else if (strcmp(argv[1], "-h") == '\0') {
	    help();
	    exit(1);
    }
    else
	strncpy(record, argv[1], 8);

    wfdbquiet();
    if ((nsig = isigopen(record, si, MAXSIG)) < 1) {
	switch (nsig) {
	  case 0:
	    printf("Sorry, record `%s' has no signals.\n", record);
	    break;
	  case -1:
	  default:
	    printf("Record `%s' can't be found.\n", record);
	    break;
	  case -2:
	    printf("The header file for record `%s' is garbled.\n", record);
	    break;
	}
	exit(2);
    }
    for (i = 0; i < nsig; i++)
	if (si[i].gain == 0.0) si[i].gain = WFDB_DEFGAIN;

    an.name = (argc > 2) ? argv[2] : "atr"; an.stat = WFDB_READ;
    if (nann = (annopen(record, &an, 1) == 0))
	getann(0, &annot);

    if (right-left >= 400)
	    sprintf(frametitle,
		"Record: %s  Annotator: %s  VIEW %.1f",
		record, (nann > 0) ? an.name : "(none)", VERSION);
    else
	sprintf(frametitle,
		"Rec %s %s  VIEW %.1f", record,
		(nann > 0) ? an.name : "", VERSION);
    
    if ((sps = sampfreq(NULL)) <= 0.) sps = WFDB_DEFFREQ;
    xppmm = (double)(right - left)/(double)xs;
    yppmm = (double)(bottom - top)/(double)ys;
    i = xs/5;   /* determine the number of 5 mm blocks we can display */
    j = xs - 5*i;       /* millimeters left over */
    left += j/2.*xppmm;
    xppsi = (xppmm * 25.) / sps;
    yppcm = yppmm * 10.;
    
    /* If specified, read the calibration file to get standard scales. */
    if (cfname == (char *)NULL && (cfname = getenv("WFDBCAL")))
	calopen(cfname);

    for (s = 0; s < nsig; s++) {
	yppadu[s] = (yppmm * 10.) / si[s].gain;
	if (getcal(si[s].desc, si[s].units, &ci) == 0 && ci.scale != 0.0)
	    yppadu[s] /= ci.scale;
    }
    if (sps >= 10.)
	trlen = i * (int)(sps/5.);
    else
	trlen = (int)(i * sps/5.);

    if (xppsi < 0.75) {
	compress = 1;

	if ((v = malloc((unsigned)(1.5*(right - left)*nsig*sizeof(int)))) ==
	    NULL) {
	    printf("Sorry, there is not enough memory available.\n");
	    exit(2);
	}
    }

    else while ((v = malloc((unsigned)(trlen*nsig*sizeof(int)))) == NULL)
	    trlen = (--i) * (int)(sps/5.);
    
    atexit(restore_mode);
    signal(SIGABRT, catch_signal);
    signal(SIGFPE, catch_signal);
    signal(SIGINT, catch_signal);

    if (!_setvideomode(vmode)) {
	printf("Video mode %d cannot be set as requested by VIEWP.\n%s",
	       vmode, evs);
	exit(2);
    }
#if _MSC_VER >= 600
    if ((viewf = getenv("VIEWF")) == NULL) {
	    char *path = getenv("PATH"), *p, *q;
	    static char s[80];

	    if (path) {
		    for (p = path; *p; p++) {
			    if (*p == 'W' && strncmp(p+1, "INDOWS", 6) == 0) {
				    q = p+7;
				    while (p > path && *(p-1) != ';')
					    p--;
				    strncpy(s, p, q-p);
				    strcat(s, "\\system\\modern.fon");
				    viewf = s;
				    break;
			    }
		    }
	    }
	    if (viewf == NULL) viewf = "modern.fon";
    }
    if (_registerfonts(viewf) < 0 ||
	   _setfont("t'modern'h12w8b") < 0)
	use_gtext = 0;
    else
	use_gtext = 1;
#endif
    _getvideoconfig(&vc);
    xppcol = vc.numxpixels/vc.numtextcols;
    ypprow = vc.numypixels/vc.numtextrows;
    tbmid = (nsig == 1) ? vc.numtextrows - 3 : vc.numtextrows/nsig;
    prow = vc.numtextrows;
    tx0 = (vc.numtextcols - strlen(frametitle))/2 + 1;
    if (tx0 < 1) tx0 = 1;
    back2.label = " <<";
    back2.left = (left + right)/2;
    back2.right = back2.left + xppcol*4;
    back1.label = "<";
    back1.left = back2.right + xppcol;
    back1.right = back1.left + xppcol*3;
    forw1.label = ">";
    forw1.left = back1.right + xppcol;
    forw1.right = forw1.left + xppcol*3;
    forw2.label = " >>";
    forw2.left = forw1.right + xppcol;
    forw2.right = forw2.left + xppcol*4;
    
    if (vc.numcolors >= 16) {
	    background_color = _BRIGHTWHITE;
	    _remappalette(15, _RED);
	    signal_color = 1;           /* blue */
	    annotation_color = 2;       /* green */
	    button_text_color = cursor_color = 4;               /* red */
	    button_color = grid_color = 7;                      /* grey */
	    grid_linestyle = signal_linestyle = 0xffff; /* solid */
	    marker_linestyle = 0x5555;  /* dotted */
    }
    else if (vc.numcolors >= 4 && vmode != _ERESNOCOLOR) {
	    background_color = _BLACK;
	    annotation_color = 1;
	    grid_color = 2;
	    button_text_color = signal_color = cursor_color = 3;
	    grid_linestyle = 0xffff;            /* solid */
	    marker_linestyle = 0x5555;  /* dotted */
	    signal_linestyle = 0xffff;  /* solid */
    }
    else {
	    signal_color = button_text_color = annotation_color = grid_color =
		    cursor_color = 1;
	    grid_linestyle = 0x1111;            /* sparse dotted */
	    marker_linestyle = 0x5555;  /* dotted */
	    signal_linestyle = 0xffff;  /* solid */
    }

    save_background_color = _getbkcolor();
    save_text_color = _gettextcolor();
    if (vc.numcolors >= 4 && vmode != _ERESNOCOLOR)
	    _setbkcolor(background_color);
    if (vc.numxpixels < 640) mpp = 2;
    
    if (findmouse() == 1) {
	use_mouse = 1;
	_setcolor(cursor_color);
	showmousecursor();
    }
    _setcolor(signal_color);

    for (s = 0; s < nsig; s++) {
	base[s] = ((2*s+1)*bottom)/(2*nsig);
	vmin[s] = 9999; vmax[s] = -9999;
    }

    if (argc > 3) {
	    strncpy(command, argv[3], 20);
	    do_command();
    }
    else
	    do_disp();

    i = 0;
    while (1) {
	    int nc;
	    
		if (kbhit()) {
			c = _getch();
			if (c == '\b' || c == '\177') {
				if (i > 0) {
					struct _rccoord tp = _gettextposition();

					_settextposition(tp.row, tp.col-1);
					_outtext(" ");
					_settextposition(tp.row, tp.col-1);
					command[--i] = '\0';
				}
			}
			else if (c == '\r') {
				do_command();
				command[i = 0] = '\0';
			}
			else {
				command[i] = c;
				command[i+1] = '\0';
				_outtext(command+i);
				if (++i >= 20) {
					do_command();
					command[i = 0] = '\0';
				}
			}
		}
		if (nann > 0 && use_mouse) {
			long t0, t1;

			if ((nc = getclicks(0)) > 0) {
				getmousestate(&ms);
				if (ms.y >= bottom - ypprow) {
					/* check for button press */
					if (back2.left <= ms.x && ms.x <= back2.right)
						tstart -= trlen;
					else if (back1.left <= ms.x && ms.x <= back1.right)
						tstart -= trlen/2;
					else if (forw1.left <= ms.x && ms.x <= forw1.right)
						tstart += trlen/2;
					else if (forw2.left <= ms.x && ms.x <= forw2.right)
						tstart += trlen;
					else
						continue;       /* ignore click if not on a button */
					if (tstart < 0L) tstart = 0L;
					else tstart = strtim(timstr(tstart));
					isigsettime(tstart);
					if (tstart == 0L) iannsettime(1L);
					else iannsettime(tstart);
					getann(0, &annot);
					do_disp();
					continue;
				}
				/* move left by nc annotations */
				t0 = t1 = tstart + (ms.x - left - 5)/xppsi;
				if (t0 < 0L) t0 = t1 = 0L;
				do {
					if (iannsettime(1L) < 0 || getann(0, &annot) < 0)
						break;
					do {
						t1 = annot.time;
					} while (getann(0, &annot) == 0 && annot.time < t0);
				/* t1 is the time of the annotation immediately before t0,
					if any */
					if (t0 <= t1) break;
					t0 = t1;
				} while (--nc > 0);
			}
			else if ((nc = getclicks(1)) > 0) {
				/* move right by nc annotations */
				getmousestate(&ms);
				t0 = tstart + (ms.x - left + 5)/xppsi;
				if (iannsettime(t0) < 0) continue;
				while (nc > 0 && getann(0, &annot) == 0) {
					nc--;
					t0 = annot.time;
				}
			}
			else
				continue;               /* no mouse clicks */
			if (t0 < tstart || t0 >= tstart + trlen) {
				if (t0 < trlen/2) t0 = trlen/2;
				tstart = strtim(timstr(t0 - trlen/2));
				isigsettime(tstart);
				if (tstart == 0L) iannsettime(1L);
				else iannsettime(tstart);
				getann(0, &annot);
				do_disp();
			}
			setmousecursor((int)((t0-tstart)*xppsi + left),
				(tby+1)*ypprow);
		}
		else if (use_mouse) {
			if ((nc = getclicks(0)) > 0) {
				getmousestate(&ms);
				if (ms.y >= bottom - ypprow) {
					/* check for button press */
					if (back2.left <= ms.x && ms.x <= back2.right)
						tstart -= trlen;
					else if (back1.left <= ms.x && ms.x <= back1.right)
						tstart -= trlen/2;
					else if (forw1.left <= ms.x && ms.x <= forw1.right)
						tstart += trlen/2;
					else if (forw2.left <= ms.x && ms.x <= forw2.right)
						tstart += trlen;
					else
						continue;       /* ignore click if not on a button */
					if (tstart < 0L) tstart = 0L;
					else tstart = strtim(timstr(tstart));
					isigsettime(tstart);
					do_disp();
					continue;
				}
			}
		}
	}
	wfdbquit();
	restore_mode();
	exit(0);
}

static char *help_strings[] = {
 "usage: view [ RECORD ANNOTATOR START ]",
 "where RECORD and ANNOTATOR specify the record and the set of annotations",
 "to be viewed, and START specifies either:",
 "  - the elapsed time (in HH:MM:SS format) from the beginning of RECORD",
 "    to the left edge of the first screenful to be viewed, or",
 "  - an annotation type (such as `V' or `F'; `view' finds the first example",
 "    and shows it centered in the first screenful)",
 "If START is omitted, `view' shows the beginning of RECORD.  If ANNOTATOR",
 "is omitted, `view' shows `atr' (reference) annotations if available.  If",
 "RECORD is omitted, `view' asks for the record name.",
 "",
 "Once `view' is running, use these commands to control it:",
 "  <ENTER>       show the next screenful",
 "  `g'           toggle grid display (0.2s x 0.5 mV)",
 "  `m'           toggle annotation marker bar display",
 "  `x'           to exit to MS-DOS",
 "or type a time or annotation type as above.",
 "",
 "If a mouse is available, click its left button on any of the controls",
 "at the bottom of the screen to move forward or backward by whole or",
 "half screenfuls, or click its left or right button in the display area",
 "to move left or right one annotation at a time.",
NULL
};

void help()
{
    int i;

    for (i = 0; help_strings[i] != NULL; i++)
	(void)fprintf(stderr, "%s\n", help_strings[i]);
}
