/* file: wview.c        G. Moody        20 January 1993
                        Last revised:     11 June 2000 (but see *** below)
   
-------------------------------------------------------------------------------
wview: view WFDB-format signals and annotations under MS Windows
Copyright (C) 2000 George B. Moody

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

#include <windows.h>
#include <string.h>
#include <wfdb/wfdb.h>
#include <wfdb/ecgcodes.h>
#include <stdlib.h>
#include "wview.h"
extern int sprintf();

#define SIGNAL_COLOR    RGB(0, 0, 255)          /* blue */
#define GRID_COLOR      RGB(192, 192, 192)      /* 25% grey */
#define PGRID_COLOR     RGB(0, 0, 0)            /* black */
#define MARKER_COLOR    RGB(255, 0, 0)          /* red */

/* Default scales and grid intervals. */
#define TSI          9  /* index (within tsi[], see below) of default time
			   scale */
#define VSI          3  /* index (within vsi[], see below) of default amplitude
			   scale */
#define GTICKSPERSEC 5  /* grid ticks per second */
#define GTICKSPERMV  2  /* grid ticks per millivolt */

/* Printing defaults. */
#define MAXCOPIES   10  /* maximum number of copies to be printed by a single
			   request */

/* Convert mm coordinates (origin at lower left of window) to window
   coordinates (origin at upper left of window).*/
#define xtr(A)  ((int)((A)*xppmm))
#define ytr(A)  ((int)(wheight - (A)*yppmm))

/* Convert signal coordinates (origin on left edge of window at baseline
   level yb, units sample intervals and A/D units) to window coordinates. */
#define xt(A)   ((int)((A)*xppsi))
#define yt(A)   ((int)(yb - (A)*ysc))

/* Similarly for printer coordinates. */
#define pxtr(A) ((int)((A)*pxppmm))
#define pytr(A) ((int)(pheight - (A)*pyppmm))
#define pxt(A)  ((int)((A)*pxppsi))
#define pyt(A)  ((int)(yb - (A)*pysc))

HANDLE hInst, hLibrary;
HPEN signal_pen;
HPEN grid_pen;
HPEN pgrid_pen;
HPEN marker_pen;
TEXTMETRIC tm;
static int cheight;                     /* y-spacing between rows of text */
static char record[20];                 /* name of record to be browsed */
static char annotator[20] = "atr";      /* name of annotator to be browsed */
static char cfname[40];                 /* name of calibration file */
static char wfdbpath[200];              /* search path for WFDB files */
static char title[80];                  /* title of main window */
static double sg[WFDB_MAXSIG];          /* signal gain * calibration scale */
static double sps = WFDB_DEFFREQ;       /* sampling frequency (Hz) */
static WFDB_Time tf = 100000L;             /* time of end of record */
static WFDB_Time t0, t1;                   /* times of left, right window edges */
static double tscale;                   /* time scale (units are mm/sec) */
static double vscale;                   /* amplitude scale (units are mm/mV) */
static WFDB_Siginfo si[WFDB_MAXSIG];    /* signal specifications */
static int nsig;                        /* number of valid signals in si[] */
static int nann;                        /* number of open annotation files */
static int tmode;             /* 0: show elapsed time; 1: show absolute time */

/* User-selectable time and amplitude scales. */
static double tsc[] = { 0.25/60., 1./60., 5./60., 25./60., 50./60., 125./60.,
                         250./60., 500./60., 12.5, 25., 50., 125., 250. };
static char *tst[] = { "0.25 mm/min", "1 mm/min", "5 mm/min", "25 mm/min",
                           "50 mm/min", "125 mm/min", "250 mm/min",
			   "500 mm/min", "12.5 mm/sec", "25 mm/sec",
                           "50 mm/sec", "125 mm/sec", "250 mm/sec" };
static int tsi = TSI;   /* index (within tsc[] and tst[]) of current tscale */
#define MAXTSI (sizeof(tsc)/sizeof(double) - 1)

static double vsc[] = { 1., 2.5, 5., 10., 20., 40., 100. };
static char *vst[] = { "1 mm/mV", "2.5 mm/mV", "5 mm/mV", "10 mm/mV",
			   "20 mm/mV", "40 mm/mV", "100 mm/mV" };
static int vsi = VSI;   /* index (within vsc[] and vst[]) of current vscale */
#define MAXVSI (sizeof(vsc)/sizeof(double) - 1)

int find_next();
int find_previous();
int open_record();
int open_annotation_file();
void paint_main_window();
void print_window();
void set_title();

int PASCAL WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow)
HANDLE hInstance;
HANDLE hPrevInstance;
LPSTR lpCmdLine;
int nCmdShow;
{
    static char *p, *q, rtemp[80], pstart[40], pdate[20];
    MSG msg;
    extern int sscanf();
    
    /* If specified, read the calibration file to get standard scales. */
    if (cfname[0] == '\0' && (p = getenv("WFDBCAL"))) {
	strncpy(cfname, p, sizeof(cfname)-1);
	calopen(cfname);
    }
    /* Set the default display scales. */
    tscale = tsc[TSI];
    vscale = vsc[VSI];
    
    rtemp[0] = '\0';
    if (lpCmdLine)
        sscanf(lpCmdLine, "%s%s%s%s", rtemp, annotator, pstart, pdate);
    
    /* Open the signal file(s) for the record to be browsed. */
    if (*rtemp) {
	/* Convert upper case characters to lower case. */
	for (p = rtemp; *p; p++)
	    if ('A' <= *p && *p <= 'Z') *p += 'a' - 'A';
	/* Was the first argument actually a header file name? */
	if (strlen(rtemp) > 4 &&
	    strcmp(rtemp+strlen(rtemp)-4, ".hea") == 0) {       /* yes */
	    /* Locate any drive/path prefix in the first argument. */
	    for (p = rtemp, q = rtemp-1; *p != '.'; p++)
		if (*p == ':' || *p == '\\') q = p;
	    /* Replace the '.' with a null to strip the '.hea' suffix. */
	    *p = '\0';
	    /* Extract the record name. */
	    strncpy(record, q+1, sizeof(record));
	    /* If there was a drive/path prefix, add it to the WFDB path. */
	    if (q >= rtemp) {
		size_t l = (size_t)(q-rtemp+1);
		
		strncpy(wfdbpath, rtemp, l);
		wfdbpath[l] = ';';
		strncpy(wfdbpath+l+1, getwfdb(), sizeof(wfdbpath)-l);
		setwfdb(wfdbpath);
	    }
	}
	else                                                    /* no */
	    strncpy(record, rtemp, sizeof(record));
	open_record();
    }
    
    /* If specified, open the annotation file for the record. */
    if (*record && *annotator && *annotator != '-')
	open_annotation_file();

    /* If specified, set the start time. */
    if (*record && *pstart) {
        if (*pstart == '[') {   /* absolute time */
            tmode = 1;
            if (*pdate) {
                strcat(pstart, " ");
                strcat(pstart, pdate);
            }
            if ((t0 = -strtim(pstart)) < 0L)
                t0 = 0L;     /* pstart precedes start of record; go to 0 */
        }
        else
            t0 = strtim(pstart);
    }

    if (!hPrevInstance)
	if (!InitApplication(hInstance))
	    return (FALSE);
    
    if (!InitInstance(hInstance, nCmdShow))
	return (FALSE);
    
    while (GetMessage(&msg, NULL, NULL, NULL)) {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }
    
    wfdbquit();
    return (msg.wParam);
}

BOOL InitApplication(hInstance)
HANDLE hInstance;
{
    WNDCLASS  wc;
    
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, "ecgicon");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH); 
    wc.lpszMenuName =  "wviewmenu";
    wc.lpszClassName = "wviewclass";
    return (RegisterClass(&wc));
}

BOOL InitInstance(hInstance, nCmdShow)
HANDLE hInstance;
int nCmdShow;
{
    HWND hWnd;
    
    hInst = hInstance;
    
    set_title();
    hWnd = CreateWindow(
			"wviewclass",
			title,
			WS_OVERLAPPEDWINDOW | WS_HSCROLL,
			0,
			0,
			GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN),
			NULL,
			NULL,
			hInstance,
			NULL
			);
    if (!hWnd)
	return (FALSE);
    
    if (*record == '\0') {
	static FARPROC lpProcChoose;
	
	lpProcChoose = MakeProcInstance(Choose, hInst);
	DialogBox(hInst, "ChooseBox", hWnd, lpProcChoose);
	FreeProcInstance(lpProcChoose);
	set_title();
	SetWindowText(hWnd, title);
    }
    
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return (TRUE);
}

static double xppmm, yppmm;     /* display resolution */
static double wwsec, whmv;      /* window size in seconds (x) and mV (y) */
static int bflag = 0;           /* draw signal baselines if non-zero */
static int gtflag = 1;          /* draw time grid if non-zero */
static int gvflag = 1;          /* draw amplitude grid if non-zero */
static int mflag = 0;           /* draw marker bars if non-zero */
static int nflag = 1;           /* show signal names if non-zero */
static int show_subtyp = 0, show_chan = 0, show_num = 0, show_aux = 0;
/* display respective fields of annotations if non-zero */
static int wwidth, wheight;     /* window size in pixels */

long FAR PASCAL MainWndProc(hWnd, message, wParam, lParam)
HWND hWnd;
UINT message;
WPARAM wParam;
LPARAM lParam;
{
    static FARPROC lpProcAbout;
    static FARPROC lpProcChoose;
    static FARPROC lpProcFind;
    static FARPROC lpProcPrint;
    static FARPROC lpProcPrintOptions;
    static FARPROC lpProcViewOptions;
    HDC hDC;
    HMENU hMenu;
    static int scrollpos = 0;   /* scroll position (0 - 2048) */
    
    switch (message) {
      case WM_COMMAND:
	hMenu = GetMenu(hWnd);
	
	switch (wParam) {
	  case IDM_OPEN:
	    lpProcChoose = MakeProcInstance(Choose, hInst);
	    if (DialogBox(hInst, "ChooseBox", hWnd, lpProcChoose))
		InvalidateRect(hWnd, NULL, TRUE);
	    FreeProcInstance(lpProcChoose);
	    set_title();
	    SetWindowText(hWnd, title);
	    break;
	  case IDM_FIND:        
	    lpProcFind = MakeProcInstance(Find, hInst);
	    if (DialogBox(hInst, "FindBox", hWnd, lpProcFind)) {
		scrollpos = (int)(2048.*t0/tf);
		SetScrollPos(hWnd, SB_HORZ, scrollpos, TRUE);
		InvalidateRect(hWnd, NULL, TRUE);
	    }
	    FreeProcInstance(lpProcFind);
	    break;
	  case IDM_NEW:
	  case IDM_SAVE:
	  case IDM_SAVEAS:
	  case IDM_UNDO:
	  case IDM_CUT:
	  case IDM_COPY:
	  case IDM_PASTE:
	  case IDM_DEL:
	    MessageBox(hWnd, "Sorry, this command is not yet implemented!",
		       "WVIEW", MB_ICONEXCLAMATION | MB_OK);
	    break;
	  case IDM_EXIT:
	    SendMessage(hWnd, WM_CLOSE, 0, 0L);
	    break;
	  case IDM_HELP_INDEX:
	    WinHelp(hWnd, "wview.hlp", HELP_CONTENTS, 0L);
	    break;
	  case IDM_HELP_CHOOSING:
	    WinHelp(hWnd, "wview.hlp", HELP_CONTEXT, (long)Choosing_Topic);
	    break;
	  case IDM_HELP_BROWSING:
	    WinHelp(hWnd, "wview.hlp", HELP_CONTEXT, (long)Browsing_Topic);
	    break;
	  case IDM_HELP_OPTIONS:
	    WinHelp(hWnd, "wview.hlp", HELP_CONTEXT, (long)Options_Topic);
	    break;
	  case IDM_HELP_SEARCHING:
	    WinHelp(hWnd, "wview.hlp", HELP_CONTEXT, (long)Searching_Topic);
	    break;
	  case IDM_HELP_PRINTING:
	    WinHelp(hWnd, "wview.hlp", HELP_CONTEXT, (long)Printing_Topic);
	    break;
	  case IDM_ABOUT:
	    lpProcAbout = MakeProcInstance(About, hInst);
	    DialogBox(hInst, "AboutBox", hWnd, lpProcAbout);
	    FreeProcInstance(lpProcAbout);
	    break;
	  case IDM_PRINT:
	    lpProcPrint = MakeProcInstance(Print, hInst);
	    DialogBox(hInst, "PrintBox", hWnd, lpProcPrint);
	    FreeProcInstance(lpProcPrint);
	    break;
	  case IDM_VIEW:
	    lpProcViewOptions = MakeProcInstance(ViewOptions, hInst);
	    DialogBox(hInst, "ViewOptionsBox", hWnd, lpProcViewOptions);
	    FreeProcInstance(lpProcViewOptions);
	    InvalidateRect(hWnd, NULL, TRUE);
	    break;
	  case IDM_PROPT:
	    lpProcPrintOptions = MakeProcInstance(PrintOptions, hInst);
	    DialogBox(hInst, "PrintOptionsBox", hWnd, lpProcPrintOptions);
	    FreeProcInstance(lpProcPrintOptions);
	    break;
	  default:
	    return (DefWindowProc(hWnd, message, wParam, lParam));
	}
	break;
	
      case WM_CREATE:
	/* Create the pen objects. */
	signal_pen = CreatePen(PS_SOLID, 0, SIGNAL_COLOR);
	grid_pen = CreatePen(PS_SOLID, 0, GRID_COLOR);
	pgrid_pen = CreatePen(PS_DOT, 0, PGRID_COLOR);
	marker_pen = CreatePen(PS_DOT, 0, MARKER_COLOR);
	
	hDC = GetDC(hWnd);
	
	/* Determine display resolution (pixels per millimeter in x and y). */
	xppmm = GetDeviceCaps(hDC,HORZRES)/(double)GetDeviceCaps(hDC,HORZSIZE);
	yppmm = GetDeviceCaps(hDC,VERTRES)/(double)GetDeviceCaps(hDC,VERTSIZE);
	
	/* Get the size characteristics of the current font.  */
	GetTextMetrics(hDC, &tm);
	cheight = tm.tmExternalLeading + tm.tmHeight;
	
	ReleaseDC(hWnd, hDC);
	break;
	
      case WM_SIZE:
	wwidth = LOWORD(lParam);
	wheight = HIWORD(lParam);
	SetScrollRange(hWnd, SB_HORZ, 0, 2048, FALSE);
	scrollpos = (int)(2048.*t0/tf);
	SetScrollPos(hWnd, SB_HORZ, scrollpos, TRUE);
	break;
	
      case WM_HSCROLL:
	switch (wParam) {
	  case SB_TOP:
	    t0 = 0L;
	    break;
	  case SB_BOTTOM:
	    t0 = tf - (WFDB_Time)(wwsec*sps);
	    break;
	  case SB_LINEUP:
	    if ((t0 -= (WFDB_Time)sps) < 0L) t0 = 0L;
	    break;
	  case SB_LINEDOWN:
	    t0 += (WFDB_Time)sps;
	    break;
	  case SB_PAGEUP:
	    if ((t0 -= (WFDB_Time)(sps*(int)wwsec)) < 0L) t0 = 0L;
	    break;
	  case SB_PAGEDOWN:
	    t0 += (WFDB_Time)(sps*(int)wwsec);
	    break;
	  case SB_THUMBTRACK:	/* events occur while dragging "thumb" */
	    /* Show the time corresponding to the current thumb position. */
	    {
		char ts[30];
		int x, y;
		
		hDC = GetDC(hWnd);
		SetBkMode(hDC, OPAQUE);
		SetTextColor(hDC, MARKER_COLOR);
		sprintf(ts, "<< %20s >>",
			timstr(-(WFDB_Time)(tf*(LOWORD(lParam)/2048.))));
		x = 200;
		y = ytr(5.) - tm.tmExternalLeading;
		TextOut(hDC, x, y, ts, strlen(ts));               
		ReleaseDC(hWnd, hDC);
	    }
	    return (NULL);	/* do not redraw the entire window */
	  case SB_THUMBPOSITION: /* event occurs when "thumb" is dropped */
	    t0 = (WFDB_Time)(tf * (LOWORD(lParam)/2048.));
	    if (t0 >= tf && tf > 0L)
		t0 = tf - (WFDB_Time)(wwsec*sps);
	    break;
	  default:
	    return (NULL);
	}
	scrollpos = (int)(2048.*t0/tf);
	SetScrollPos(hWnd, SB_HORZ, scrollpos, TRUE);
	InvalidateRect(hWnd, NULL, TRUE);
	break;
	
      case WM_KEYDOWN:
	switch (wParam) {
	  case VK_HOME:         /* <Home> key */
	    SendMessage(hWnd, WM_HSCROLL, SB_TOP, 0L);
	    break;
	  case VK_END:          /* <End> key */
	    SendMessage(hWnd, WM_HSCROLL, SB_BOTTOM, 0L);
	    break;
	  case VK_LEFT:         /* <left arrow> */
	    SendMessage(hWnd, WM_HSCROLL, SB_LINEUP, 0L);
	    break;
	  case VK_RIGHT:        /* <right arrow> */
	    SendMessage(hWnd, WM_HSCROLL, SB_LINEDOWN, 0L);
	    break;
	  case VK_PRIOR:        /* <Page Up> */
	    SendMessage(hWnd, WM_HSCROLL, SB_PAGEUP, 0L);
	    break;
	  case VK_NEXT:         /* <Page Down> */
	    SendMessage(hWnd, WM_HSCROLL, SB_PAGEDOWN, 0L);
	    break;
	  case VK_BACK:         /* <backspace> */
	    switch (find_previous()) {
	      case 1:   /* target found, reset scrollbar and redraw */
		scrollpos = (int)(2048.*t0/tf);
		SetScrollPos(hWnd, SB_HORZ, scrollpos, TRUE);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	      case 0:   /* not found, ignore */
		break;
	      case -1:  /* no target defined, handle as for page up */
		SendMessage(hWnd, WM_HSCROLL, SB_PAGEUP, 0L);
		break;
	    }
	    break;
	  case VK_RETURN:       /* <enter> */
	    switch (find_next()) {
	      case 1:   /* target found, reset scrollbar and redraw */
		scrollpos = (int)(2048.*t0/tf);
		SetScrollPos(hWnd, SB_HORZ, scrollpos, TRUE);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	      case 0:   /* not found, ignore */
		break;
	      case -1:  /* no target defined, handle as for page down */
		SendMessage(hWnd, WM_HSCROLL, SB_PAGEDOWN, 0L);
		break;
	    }
	    break;
	  case VK_UP:   /* <up arrow>: zoom in (increase time scale) */
	    if (tsi < MAXTSI) {
		tscale = tsc[++tsi];
		/* Adjust time of left edge such that the time of
		   the center of the window will be unchanged. */
		t0 += (WFDB_Time)(sps*wwidth/(2.*xppmm)*
			       (1./tsc[tsi-1] - 1./tscale));
		InvalidateRect(hWnd, NULL, TRUE);
	    }
	    break;
	  case VK_DOWN: /* <down arrow>: zoom out (decrease time scale) */
	    if (tsi > 0) {
		tscale = tsc[--tsi];
		/* Adjust time of left edge such that the time of
		   the center of the window will be unchanged. */
		t0 += (WFDB_Time)(sps*wwidth/(2.*xppmm)*
			       (1./tsc[tsi+1] - 1./tscale));
		if (t0 < 0L) t0 = 0L;
		InvalidateRect(hWnd, NULL, TRUE);
	    }
	    break;
	  default:
	    break;
	}
	
      case WM_CHAR:
	switch (wParam) {
	  case '+':     /* increase amplitude scale */
	    if (vsi < MAXVSI) {
		vscale = vsc[++vsi];
		InvalidateRect(hWnd, NULL, TRUE);
	    }
	    break;
	  case '-':     /* decrease amplitude scale */
	    if (vsi > 0) {
		vscale = vsc[--vsi];
		InvalidateRect(hWnd, NULL, TRUE);
	    }
	    break;
	  default:
	    break;
	}
	break;
	
      case WM_PAINT:
	paint_main_window(hWnd);
	break;
	
      case WM_DESTROY:
	DeleteObject(signal_pen);
	DeleteObject(grid_pen);
	DeleteObject(pgrid_pen);
	DeleteObject(marker_pen);
	if (hLibrary >= 32) FreeLibrary(hLibrary);
	WinHelp(hWnd, "wview.hlp", HELP_QUIT, NULL);
	PostQuitMessage(0);
	break;
	
      default:
	return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (NULL);
}

BOOL FAR PASCAL Choose(hDlg, message, wParam, lParam)
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
{
    static char rtemp[200], *r, *s, *t;
    
    switch (message) {
      case WM_INITDIALOG:
	SetDlgItemText(hDlg, IDD_RECORD, record);
	SetDlgItemText(hDlg, IDD_ANNOTATOR, annotator);
	if ((t = getwfdb()) != wfdbpath) {
	    strncpy(wfdbpath, t, sizeof(wfdbpath)-1);
	    setwfdb(wfdbpath);	/* *** 16 July 2003 (GBM).  This call is
				   necessary for WFDB 10.3.9 and later, in
				   which setwfdb keeps its own copy of the
				   WFDB path. */
	}
	SetDlgItemText(hDlg, IDD_WFDBPATH, wfdbpath);
	SetDlgItemText(hDlg, IDD_WFDBCAL, cfname);
	return (TRUE);
	
      case WM_COMMAND:
	switch (wParam) {
	  case IDOK:
	    GetDlgItemText(hDlg, IDD_RECORD, rtemp, sizeof(rtemp)-1);
	    for (t = rtemp; t < rtemp+20 && *t == ' '; t++) ;
	    for (r = record; t < rtemp+20 && *t && *t != ' '; *r++ = *t++) ;
	    if (r > record) *r = '\0';
	    GetDlgItemText(hDlg, IDD_ANNOTATOR, rtemp, sizeof(rtemp)-1);
	    for (t = rtemp; t < rtemp+20 && *t == ' '; t++) ;
	    for (s = annotator; t < rtemp+20 && *t && *t != ' '; *s++ = *t++) ;
	    if (s > annotator) *s = '\0';
	    GetDlgItemText(hDlg, IDD_WFDBPATH, rtemp, sizeof(rtemp)-1);
	    for (t = rtemp; t < rtemp+200 && *t == ' '; t++) ;
	    if (strcmp(wfdbpath, t)) {
		strcpy(wfdbpath, t);
		setwfdb(wfdbpath);
	    }
	    GetDlgItemText(hDlg, IDD_WFDBCAL, rtemp, sizeof(rtemp)-1);
	    for (t = rtemp; t < rtemp+200 && *t == ' '; t++) ;
	    if (strcmp(cfname, t)) {
		strcpy(cfname, t);
		calopen(cfname);
	    }
	    if (r > record || s >  annotator) { open_record(); nann = 0; }
	    if (s > annotator) open_annotation_file();
	    EndDialog(hDlg, 1);
	    return (TRUE);
	    
	  case IDCANCEL:
	    EndDialog(hDlg, 0);
	    return (TRUE);
	    
	  default:
	    return (FALSE);
	}
	break;
    }
    return (FALSE);
}

int strnsubtyp(s)
char *s;
{
    int i = 0;
    
    while (*s) {
	i <<= 1;
	switch (*s) {
	  case 'n':     i |= 1; break;
	  case 'u':     i |= 0x11; break;
	  case 'U':     return (-1);
	  default:      break;
	}
	s++;
    }
    return (i);
}

static int match_anntyp, match_subtyp, match_chan, match_num, match_aux;

void asparse(char *s, WFDB_Annotation *annp)
{
    match_anntyp = match_subtyp = match_chan = match_num = match_aux = FALSE;
    if ((annp->anntyp = strann(s)) == NOTQRS) {
	switch (*s) {
	  case 'c':     /* search for a signal quality annotation */
	  case 'n':
	  case 'u':
	  case 'U':
	    annp->anntyp = NOISE;
	    annp->subtyp = strnsubtyp(s);
	    match_anntyp = match_subtyp = TRUE;
	    break;
	  case '*':     /* accept any annotation as a match */
	  case '\0':
	    break;
	  default:      /* match aux field only */
	    *(s-1) = strlen(s);
	    annp->aux = s-1;
	    match_aux = TRUE;
	    break;
	}
    }
    else
	match_anntyp = TRUE;
}

int match(WFDB_Annotation *annp, WFDB_Annotation*temp)
{
    return ((!match_anntyp || annp->anntyp == temp->anntyp) &&
	    (!match_subtyp || annp->subtyp == temp->subtyp) &&
	    /*          (!match_chan   || annp->chan   == temp->chan  ) &&
			(!match_num    || annp->num    == temp->num   ) &&  */
	    (!match_aux    || (annp->aux &&
			       strncmp(annp->aux+1,temp->aux+1,*(temp->aux)) == 0)));
}

static char fns[20], fps[20];

int find_next()
{
    WFDB_Annotation annot, target;
    
    if (fns[1] == '\0') return (-1);
    asparse(fns+1, &target);
    while (getann(0, &annot) == 0)
	if (match(&annot, &target) && annot.time > t1) {
	    t0 = annot.time - (t1-t0)/2L;
	    if (t0 < 0L) t0 = 0L;
	    return (1);
	}
    return (0);
}

int find_previous()
{
    WFDB_Annotation annot, target;
    WFDB_Time ts = -1L;
    
    if (fps[1] == '\0') return (-1);
    asparse(fps+1, &target);
    iannsettime(0L);
    while (getann(0, &annot) == 0 && annot.time < t0)
	if (match(&annot, &target))
	    ts = annot.time;
    if (ts >= 0L) {
	t0 = ts - (t1-t0)/2L;
	if (t0 < 0L) t0 = 0L;
	return (1);
    }
    return (0);
}

BOOL FAR PASCAL Find(hDlg, message, wParam, lParam)
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
{
    static char rcs[20];
    static WORD find_mode = IDD_RECENTER;
    
    switch (message) {
      case WM_INITDIALOG:
	CheckRadioButton(hDlg, IDD_FINDN, IDD_RECENTER, find_mode);
	SetDlgItemText(hDlg, IDD_FNTYPE, fns+1);
	SetDlgItemText(hDlg, IDD_FPTYPE, fps+1);
	SetDlgItemText(hDlg, IDD_RCTIME, rcs);
	return (TRUE);
	
      case WM_COMMAND:
	switch (wParam) {
	  case IDD_FINDN:
	  case IDD_FINDP:
	  case IDD_RECENTER:
	    CheckRadioButton(hDlg, IDD_FINDN, IDD_RECENTER, find_mode=wParam);
	    break;
	  case IDOK:
	    switch (find_mode) {
	      case IDD_FINDN:
		GetDlgItemText(hDlg, IDD_FNTYPE, fns+1, sizeof(fns)-2);
		if (find_next() > 0) {
		    EndDialog(hDlg, 1);
		    return (TRUE);
		}
		return (FALSE);
		
	      case IDD_FINDP:
		GetDlgItemText(hDlg, IDD_FPTYPE, fps+1, sizeof(fps)-2);
		if (find_previous() > 0) {
		    EndDialog(hDlg, 1);
		    return (TRUE);
		}
		return (FALSE);
		
	      case IDD_RECENTER:
		GetDlgItemText(hDlg, IDD_RCTIME, rcs, sizeof(rcs)-1);
		t0 = strtim(rcs) - (t1-t0)/2L;
		if (t0 < 0L) t0 = 0L;
		EndDialog(hDlg, 1);
		return (TRUE);
	    }
	    break;
	  case IDCANCEL:
	    EndDialog(hDlg, 0);
	    return (TRUE);
	    
	  default:
	    return (FALSE);
	}
	break;
    }
    return (FALSE);
}

BOOL FAR PASCAL About(hDlg, message, wParam, lParam)
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
{
    switch (message) {
      case WM_INITDIALOG:
	return (TRUE);
	
      case WM_COMMAND:
	if (wParam == IDOK || wParam == IDCANCEL) {
	    EndDialog(hDlg, TRUE);
	    return (TRUE);
	}
	break;
    }
    return (FALSE);
}

HDC GetPrinterDC(char *device)
{
    char devspec[80], *driver, *output;
    
    GetProfileString("devices", device, "", devspec, sizeof(devspec));
    if ((driver = strtok(devspec, ", ")) &&
	(output = strtok(NULL, ", ")))
	return (CreateDC(driver, device, output, NULL));
    else
	return ((HDC)0);
}

typedef VOID (FAR PASCAL *DEVMODEPROC) (HWND, HANDLE, LPSTR, LPSTR);

void printer_setup(HWND hWnd, char *printer)
{
    static char driver[16], drivername[20], *output;
    DEVMODEPROC devmodeproc;
    
    GetProfileString("devices", printer, "", driver, sizeof(driver));
    output = strtok(driver, ", ");
    sprintf(drivername, "%s.DRV", driver);
    if (hLibrary >= 32) FreeLibrary(hLibrary);
    hLibrary = LoadLibrary(drivername);
    if (hLibrary >= 32) {
	devmodeproc = (DEVMODEPROC)GetProcAddress(hLibrary, "DEVICEMODE");
	devmodeproc(hWnd, hLibrary, printer, output);
    }
}

WFDB_Time pt0, pt1;
WORD collate = 1, ncopies = 1, print_to_file = 0;

BOOL FAR PASCAL Print(hDlg, message, wParam, lParam)
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
{
    unsigned int i;
    static char *adp, alldev[4096], curdev[80], *printer, prtst[40],
    *driver, *output, cprdevstr[80], *tprinter;
    static FARPROC lpProcPrintOptions;
    static WORD print_range = IDD_PRWIN, tprint_range = IDD_PRWIN;
    HWND hlist;
    
    switch (message) {
      case WM_INITDIALOG:
	CheckRadioButton(hDlg, IDD_PRREC, IDD_PRSEG, print_range);
	EnableWindow(GetDlgItem(hDlg, IDD_PRFLABEL), print_range == IDD_PRSEG);
	EnableWindow(GetDlgItem(hDlg, IDD_PRFROM), print_range == IDD_PRSEG);
	EnableWindow(GetDlgItem(hDlg, IDD_PRTLABEL), print_range == IDD_PRSEG);
	EnableWindow(GetDlgItem(hDlg, IDD_PRTO), print_range == IDD_PRSEG);
	if (print_range == IDD_PRSEG) {
	    SetDlgItemText(hDlg, IDD_PRFROM, mstimstr(pt0));
	    SetDlgItemText(hDlg, IDD_PRTO, mstimstr(pt1));
	}
	CheckDlgButton(hDlg, IDD_PFILE, print_to_file);
	CheckDlgButton(hDlg, IDD_PCOLLATE, collate);
	SetDlgItemInt(hDlg, IDD_PCOPIES, ncopies, FALSE);
	hlist = GetDlgItem(hDlg, IDD_PPRINTER);
	if (printer == NULL) {
	    GetProfileString("windows", "device", "...", curdev,
			     sizeof(curdev));
	    if ((printer = strtok(curdev, ",")) &&
		(driver = strtok(NULL, ", ")) &&
		(output = strtok(NULL, ", ")))
		sprintf(cprdevstr, "%s on %s", printer, output);
	}
	if (*cprdevstr) {
	    SendMessage(hlist, CB_ADDSTRING, 0, (LONG)(LPSTR)cprdevstr);
	    SendMessage(hlist, CB_SETCURSEL, 0, 0L);
	}
	GetProfileString("devices", NULL, "", alldev, sizeof(alldev));
	for (adp = alldev; *adp; adp += strlen(adp) + 1) {
	    char devspec[80], prdevstr[80];
	    
	    GetProfileString("devices", adp, "", devspec, sizeof(devspec));
	    if ((driver = strtok(devspec, ", ")) &&
		(output = strtok(NULL, ", "))) {
		sprintf(prdevstr, "%s on %s", adp, output);
		if (strcmp(prdevstr, cprdevstr))
		    SendMessage(hlist, CB_ADDSTRING, 0, (LONG)(LPSTR)prdevstr);
	    }
	}
	return (TRUE);
	
      case WM_COMMAND:
	switch (wParam) {
	  case IDD_POPTIONS:
	    lpProcPrintOptions = MakeProcInstance(PrintOptions, hInst);
	    DialogBox(hInst, "PrintOptionsBox", hDlg, lpProcPrintOptions);
	    FreeProcInstance(lpProcPrintOptions);
	    break;
	  case IDD_PRSEG:
	    if (!IsWindowEnabled(GetDlgItem(hDlg, IDD_PRFROM))) {
		EnableWindow(GetDlgItem(hDlg, IDD_PRFLABEL), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDD_PRFROM), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDD_PRTLABEL), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDD_PRTO), TRUE);
	    }
	    CheckRadioButton(hDlg, IDD_PRREC, IDD_PRSEG, tprint_range=wParam);
	    break;
	  case IDD_PRWIN:
	  case IDD_PRREC:
	    if (IsWindowEnabled(GetDlgItem(hDlg, IDD_PRFROM))) {
		EnableWindow(GetDlgItem(hDlg, IDD_PRFLABEL), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDD_PRFROM), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDD_PRTLABEL), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDD_PRTO), FALSE);
	    }
	    CheckRadioButton(hDlg, IDD_PRREC, IDD_PRSEG, tprint_range=wParam);
	    break;
	  case IDD_PSETUP:
	    GetDlgItemText(hDlg, IDD_PPRINTER, curdev, sizeof(curdev));
	    for (adp = alldev; *adp; adp += strlen(adp) + 1)
		if (strncmp(curdev, adp, strlen(adp)) == 0)
		    tprinter = adp;
	    printer_setup(hDlg, tprinter);
	    break;
	  case IDOK:
	    GetDlgItemText(hDlg, IDD_PPRINTER, curdev, sizeof(curdev));
	    if (curdev != cprdevstr) strcpy(cprdevstr, curdev);
	    for (adp = alldev; *adp; adp += strlen(adp) + 1)
		if (strncmp(curdev, adp, strlen(adp)) == 0)
		    printer = adp;
	    print_to_file = IsDlgButtonChecked(hDlg, IDD_PFILE);
	    collate = IsDlgButtonChecked(hDlg, IDD_PCOLLATE);
	    switch (print_range = tprint_range) {
	      case IDD_PRREC:
		pt0 = 0L;
		pt1 = tf;
		break;
	      case IDD_PRWIN:
	      default:
		pt0 = t0;
		pt1 = t1;
		break;
	      case IDD_PRSEG:
		GetDlgItemText(hDlg, IDD_PRFROM, prtst, sizeof(prtst));
		pt0 = strtim(prtst);
		GetDlgItemText(hDlg, IDD_PRTO, prtst, sizeof(prtst));
		pt1 = strtim(prtst);
		if (pt0 < 0L) pt0 = -pt0;
		if (pt1 < 0L) pt1 = -pt1;
		if (pt0 >= pt1) pt1 = pt0 + t1-t0;
		break;
	    }
	    ncopies = GetDlgItemInt(hDlg, IDD_PCOPIES, NULL, FALSE);
	    if (ncopies > MAXCOPIES) ncopies = MAXCOPIES;
	    else if (ncopies < 1) ncopies = 1;
	    for (i = 0; i < ncopies; i++)
		print_window(hDlg, printer);
	    EndDialog(hDlg, TRUE);
	    return (TRUE);
	    break;
	  case IDCANCEL:
	    EndDialog(hDlg, TRUE);
	    return (TRUE);
	    break;
	}
    }
    return (FALSE);
}

static char pcomments[80];
static double ptscale = 25.0, pvscale = 10.0;
static int pbflag = 1, pgtflag = 1, pgvflag = 1, pmflag = 0, pnflag = 1,
    pshow_subtyp = 0, pshow_chan = 0, pshow_num = 0, pshow_aux = 0,
    ptsi = TSI, pvsi = VSI, ptmode = 0;

void copy_display_options()
{
    ptscale = tsc[ptsi = tsi];  /* time scale */
    pvscale = vsc[pvsi = vsi];  /* amplitude scale */
    pbflag = bflag;             /* signal baselines */
    pgtflag = gtflag;           /* time grid */
    pgvflag = gvflag;           /* amplitude grid */
    pmflag = mflag;             /* markers */
    pnflag = nflag;             /* signal names */
    pshow_subtyp = show_subtyp; /* annotation subtypes */
    pshow_chan = show_chan;     /* annotation chan fields */
    pshow_num = show_num;       /* annotation num fields */
    pshow_aux = show_aux;       /* annotation aux fields */
    ptmode = tmode;             /* time display mode */
}

BOOL FAR PASCAL PrintOptions(hDlg, message, wParam, lParam)
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
{
    int i;
    static int use_display_options = 1;
    HWND hlist;
    
    switch (message) {
      case WM_INITDIALOG:
	CheckDlgButton(hDlg, IDD_PODEF, use_display_options);
	if (use_display_options) copy_display_options();
	hlist = GetDlgItem(hDlg, IDD_POSTIME);
	for (i = 0; i <= MAXTSI; i++)
	    SendMessage(hlist, CB_ADDSTRING, 0, (LONG)(LPSTR)tst[i]);
	SendMessage(hlist, CB_SETCURSEL, ptsi, 0L);
	hlist = GetDlgItem(hDlg, IDD_POSAMP);
	for (i = 0; i <= MAXVSI; i++)
	    SendMessage(hlist, CB_ADDSTRING, 0, (LONG)(LPSTR)vst[i]);
	SendMessage(hlist, CB_SETCURSEL, pvsi, 0L);
	CheckDlgButton(hDlg, IDD_POGTIME, pgtflag);
	CheckDlgButton(hDlg, IDD_POGAMP, pgvflag);
	CheckDlgButton(hDlg, IDD_POAMARK, pmflag);
	CheckDlgButton(hDlg, IDD_POASUB, pshow_subtyp);
	CheckDlgButton(hDlg, IDD_POACHAN, pshow_chan);
	CheckDlgButton(hDlg, IDD_POANUM, pshow_num);
	CheckDlgButton(hDlg, IDD_POAAUX, pshow_aux);
	CheckDlgButton(hDlg, IDD_POOSB, pbflag);
	CheckDlgButton(hDlg, IDD_POOSN, pnflag);
        CheckDlgButton(hDlg, IDD_POTMODE, ptmode);
	return (TRUE);
	
      case WM_COMMAND:
	switch (wParam) {
	  case IDD_PODEF:
	    use_display_options = IsDlgButtonChecked(hDlg, IDD_PODEF);
	    if (use_display_options) {
		copy_display_options();
		hlist = GetDlgItem(hDlg, IDD_POSTIME);
		SendMessage(hlist, CB_SETCURSEL, ptsi, 0L);
		hlist = GetDlgItem(hDlg, IDD_POSAMP);
		SendMessage(hlist, CB_SETCURSEL, pvsi, 0L);
		CheckDlgButton(hDlg, IDD_POGTIME, pgtflag);
		CheckDlgButton(hDlg, IDD_POGAMP, pgvflag);
		CheckDlgButton(hDlg, IDD_POAMARK, pmflag);
		CheckDlgButton(hDlg, IDD_POASUB, pshow_subtyp);
		CheckDlgButton(hDlg, IDD_POACHAN, pshow_chan);
		CheckDlgButton(hDlg, IDD_POANUM, pshow_num);
		CheckDlgButton(hDlg, IDD_POAAUX, pshow_aux);
		CheckDlgButton(hDlg, IDD_POOSB, pbflag);
		CheckDlgButton(hDlg, IDD_POOSN, pnflag);
                CheckDlgButton(hDlg, IDD_POTMODE, ptmode);
	    }
	    break;
	  case IDD_POSTIME:
	  case IDD_POSAMP:
	  case IDD_POGTIME:
	  case IDD_POGAMP:
	  case IDD_POAMARK:
	  case IDD_POASUB:
	  case IDD_POACHAN:
	  case IDD_POANUM:
	  case IDD_POAAUX:
	  case IDD_POOSB:
	  case IDD_POOSN:
          case IDD_POTMODE:
	    CheckDlgButton(hDlg, IDD_PODEF, use_display_options = 0);
	    break;
	  case IDD_POHELP:
	    WinHelp(hDlg, "wview.hlp", HELP_CONTEXT, (long)Options_Topic);
	    break;
	  case IDOK:
	    hlist = GetDlgItem(hDlg, IDD_POSTIME);
	    i = (int)SendMessage(hlist, CB_GETCURSEL, 0, 0L);
	    if (0 <= i && i <= MAXTSI)
		ptscale = tsc[ptsi = i];
	    hlist = GetDlgItem(hDlg, IDD_POSAMP);
	    i = (int)SendMessage(hlist, CB_GETCURSEL, 0, 0L);
	    if (0 <= i && i <= MAXVSI)
		pvscale = vsc[pvsi = i];
	    pgtflag = IsDlgButtonChecked(hDlg, IDD_POGTIME);
	    pgvflag = IsDlgButtonChecked(hDlg, IDD_POGAMP);
	    pmflag = IsDlgButtonChecked(hDlg, IDD_POAMARK);
	    pshow_subtyp = IsDlgButtonChecked(hDlg, IDD_POASUB);
	    pshow_chan = IsDlgButtonChecked(hDlg, IDD_POACHAN);
	    pshow_num = IsDlgButtonChecked(hDlg, IDD_POANUM);
	    pshow_aux = IsDlgButtonChecked(hDlg, IDD_POAAUX);
	    pbflag = IsDlgButtonChecked(hDlg, IDD_POOSB);
	    pnflag = IsDlgButtonChecked(hDlg, IDD_POOSN);
            ptmode = IsDlgButtonChecked(hDlg, IDD_POTMODE);
	    EndDialog(hDlg, TRUE);
	    return (TRUE);
	  case IDCANCEL:
	    EndDialog(hDlg, TRUE);
	    return (TRUE);
	}
	break;
    }
    return (FALSE);
}

void copy_default_options()
{
    tscale = tsc[tsi = TSI];    /* time scale */
    vscale = vsc[vsi = VSI];    /* amplitude scale */
    bflag = 0;                  /* signal baselines */
    gtflag = 1;                 /* time grid */
    gvflag = 1;                 /* amplitude grid */
    mflag = 0;                  /* markers */
    nflag = 1;                  /* signal names */
    show_subtyp = 0;            /* annotation subtypes */
    show_chan = 0;              /* annotation chan fields */
    show_num = 0;               /* annotation num fields */
    show_aux = 0;               /* annotation aux fields */
}

BOOL FAR PASCAL ViewOptions(hDlg, message, wParam, lParam)
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
{
    int i;
    static int use_default_options = 1;
    HWND hlist;
    
    switch (message) {
      case WM_INITDIALOG:
	CheckDlgButton(hDlg, IDD_VODEF, use_default_options);
	if (use_default_options) copy_default_options();
	hlist = GetDlgItem(hDlg, IDD_VOSTIME);
	for (i = 0; i <= MAXTSI; i++)
	    SendMessage(hlist, CB_ADDSTRING, 0, (LONG)(LPSTR)tst[i]);
	SendMessage(hlist, CB_SETCURSEL, tsi, 0L);
	hlist = GetDlgItem(hDlg, IDD_VOSAMP);
	for (i = 0; i <= MAXVSI; i++)
	    SendMessage(hlist, CB_ADDSTRING, 0, (LONG)(LPSTR)vst[i]);
	SendMessage(hlist, CB_SETCURSEL, vsi, 0L);
	CheckDlgButton(hDlg, IDD_VOGTIME, gtflag);
	CheckDlgButton(hDlg, IDD_VOGAMP, gvflag);
	CheckDlgButton(hDlg, IDD_VOAMARK, mflag);
	CheckDlgButton(hDlg, IDD_VOASUB, show_subtyp);
	CheckDlgButton(hDlg, IDD_VOACHAN, show_chan);
	CheckDlgButton(hDlg, IDD_VOANUM, show_num);
	CheckDlgButton(hDlg, IDD_VOAAUX, show_aux);
	CheckDlgButton(hDlg, IDD_VOOSB, bflag);
	CheckDlgButton(hDlg, IDD_VOOSN, nflag);
        CheckDlgButton(hDlg, IDD_VOTMODE, tmode);
	return (TRUE);
	
      case WM_COMMAND:
	switch (wParam) {
	  case IDD_VODEF:
	    use_default_options = IsDlgButtonChecked(hDlg, IDD_VODEF);
	    if (use_default_options) {
		copy_default_options();
		hlist = GetDlgItem(hDlg, IDD_VOSTIME);
		SendMessage(hlist, CB_SETCURSEL, tsi, 0L);
		hlist = GetDlgItem(hDlg, IDD_VOSAMP);
		SendMessage(hlist, CB_SETCURSEL, vsi, 0L);
		CheckDlgButton(hDlg, IDD_VOGTIME, gtflag);
		CheckDlgButton(hDlg, IDD_VOGAMP, gvflag);
		CheckDlgButton(hDlg, IDD_VOAMARK, mflag);
		CheckDlgButton(hDlg, IDD_VOASUB, show_subtyp);
		CheckDlgButton(hDlg, IDD_VOACHAN, show_chan);
		CheckDlgButton(hDlg, IDD_VOANUM, show_num);
		CheckDlgButton(hDlg, IDD_VOAAUX, show_aux);      
		CheckDlgButton(hDlg, IDD_VOOSB, bflag);
		CheckDlgButton(hDlg, IDD_VOOSN, nflag);
                CheckDlgButton(hDlg, IDD_VOTMODE, tmode);
	    }
	    break;
	  case IDD_VOSTIME:
	  case IDD_VOSAMP:
	  case IDD_VOGTIME:
	  case IDD_VOGAMP:
	  case IDD_VOAMARK:
	  case IDD_VOASUB:
	  case IDD_VOACHAN:
	  case IDD_VOANUM:
	  case IDD_VOAAUX:
	  case IDD_VOOSB:
	  case IDD_VOOSN:
          case IDD_VOTMODE:
	    CheckDlgButton(hDlg, IDD_VODEF, use_default_options = 0);
	    break;
	  case IDD_VOHELP:
	    WinHelp(hDlg, "wview.hlp", HELP_CONTEXT, (long)Options_Topic);
	    break;
	  case IDOK:
	    hlist = GetDlgItem(hDlg, IDD_VOSTIME);
	    i = (int)SendMessage(hlist, CB_GETCURSEL, 0, 0L);
	    if (0 <= i && i <= MAXTSI)
		tscale = tsc[tsi = i];
	    hlist = GetDlgItem(hDlg, IDD_VOSAMP);
	    i = (int)SendMessage(hlist, CB_GETCURSEL, 0, 0L);
	    if (0 <= i && i <= MAXVSI)
		vscale = vsc[vsi = i];
	    gtflag = IsDlgButtonChecked(hDlg, IDD_VOGTIME);
	    gvflag = IsDlgButtonChecked(hDlg, IDD_VOGAMP);
	    mflag = IsDlgButtonChecked(hDlg, IDD_VOAMARK);
	    show_subtyp = IsDlgButtonChecked(hDlg, IDD_VOASUB);
	    show_chan = IsDlgButtonChecked(hDlg, IDD_VOACHAN);
	    show_num = IsDlgButtonChecked(hDlg, IDD_VOANUM);
	    show_aux = IsDlgButtonChecked(hDlg, IDD_VOAAUX);
	    bflag = IsDlgButtonChecked(hDlg, IDD_VOOSB);
	    nflag = IsDlgButtonChecked(hDlg, IDD_VOOSN);
            tmode = IsDlgButtonChecked(hDlg, IDD_VOTMODE);
	    EndDialog(hDlg, TRUE);
	    return (TRUE);
	  case IDCANCEL:
	    EndDialog(hDlg, TRUE);
	    return (TRUE);
	}
	break;
    }
    return (FALSE);
}

open_record()
{
    char *s;
    int i;
    WFDB_Calinfo ci;
    
    /* Save the current time in string format. */
    if (t0 > 0L) s = timstr(t0);
    wfdbquit();
    
    /* Open the signal file(s) for the record to be browsed. */
    nsig = isigopen(record, si, WFDB_MAXSIG);
    
    /* Recalculate the current time (necessary if the sampling frequencies
       of the new record and the old record differ). */
    if (t0 > 0L) t0 = strtim(s);
    
    /* Get the sampling frequency. */
    if ((sps = sampfreq(NULL)) <= 0.) sps = WFDB_DEFFREQ;
    
    /* Get the record length.  Since the scrollbar units are determined
       from the record length, this must be non-zero;  if the length of
       the chosen record is undefined or implausibly short, assume it is
       30 minutes. */
    if ((tf = strtim("e")) <= 0L && (tf = si[0].nsamp) < sps*10.) 
	tf = (long)(sps*1800.);
    
    for (i = 0; i < nsig; i++) {
	sg[i] = (si[i].gain != 0.0) ? si[i].gain : WFDB_DEFGAIN;
	if (getcal(si[i].desc, si[i].units, &ci) == 0 && ci.scale != 0.0)
	    sg[i] *= ci.scale;
    }
    
    return (nsig);
}

int open_annotation_file()
{
    static WFDB_Anninfo ai;
    
    ai.name = annotator; ai.stat = WFDB_READ;
    return (nann = annopen(record, &ai, 1) + 1);
}

void set_title()
{
    if (*record && nann > 0)
	sprintf(title, "WVIEW -- Record %s, Annotator %s",
		record, annotator);
    else if (*record)
	sprintf(title, "WVIEW -- Record %s", record);
    else
	sprintf(title, "WVIEW");
}

void paint_main_window(hWnd)
HWND hWnd;
{
    char *p, ts[30];
    double xppsi, yppadu[WFDB_MAXSIG], ysc;
    DWORD dwextent;
    HDC hDC;
    HPEN hOldPen;
    int base[WFDB_MAXSIG], compress = 0, j, s, slen, tbhigh, tblow, tbmid,
    *v, *vp, vmax[WFDB_MAXSIG], vmin[WFDB_MAXSIG], v0, x, y, yb;
    long i, trlen;
    PAINTSTRUCT ps;
    
    /* Set up a display context to begin painting. */
    hDC = BeginPaint(hWnd, &ps);
    
    /* Determine the window size in sec and mV. */
    wwsec = wwidth/(tscale*xppmm);
    whmv = wheight/(vscale*yppmm);
    
    /* Calculate the time scale in pixels per sample interval. */
    xppsi = xppmm * tscale / sps;
    
    /* Draw in transparent mode (do not clear spaces around letters,
       etc., to white). */
    SetBkMode(hDC, TRANSPARENT);
    
    /* Draw the grid, if requested. */
    hOldPen = SelectObject(hDC, grid_pen);
    if (gtflag) {
	double gdx = xppmm*tscale/GTICKSPERSEC;
	int i;
	
	for (i = 1, x = (int)gdx; x < wwidth; i++, x = (int)(i*gdx)) {
	    MoveTo(hDC, x, 0);
	    LineTo(hDC, x, wheight - 1);
	}
    }
    if (gvflag) {
	double gdy = yppmm*vscale/GTICKSPERMV;
	int i;
	
	for (i = 1, y = (int)gdy; y < wheight; i++, y = (int)(i*gdy)){
	    MoveTo(hDC, 0, y);
	    LineTo(hDC, wwidth - 1, y);
	}
    }
    
    if (nsig > 0 || nann > 0) {
	/* Set color for time stamps and signal names. */
	SetTextColor(hDC, SIGNAL_COLOR);
	
	/* Show the starting time in the lower left corner. */
        slen = sprintf(ts, "%s", timstr(tmode ? -t0 : t0));
	for (p = ts; *p == ' '; p++, slen--)
	    ;   /* discard initial spaces, if any */
	x = xtr(2.);
	y = ytr(5.) - tm.tmExternalLeading;
	TextOut(hDC, x, y, p, slen);
	
	/* Determine the ending time. */
	t1 = t0 + (WFDB_Time)(sps*(int)wwsec);
	
	/* Show the ending time in the lower right corner. */
        slen = sprintf(ts, "%s", timstr(tmode ? -t1 : t1));
	for (p = ts; *p == ' '; p++, slen--)
	    ;   /* discard initial spaces, if any */
	dwextent = GetTextExtent(hDC, p, slen);
	x = xt(sps*(int)wwsec) - LOWORD(dwextent) - xtr(2.);
	TextOut(hDC, x, y, p, slen);
    }
    
    if (nsig < 1)
	tbmid = wheight/2;      /* Set standard ordinate for annotations. */
    else {      /* Draw signals. */
	/* Determine signal amplitude scales and baselines. */
	for (s = 0; s < nsig; s++) {
	    yppadu[s] = (yppmm * vscale) / sg[s];
	    base[s] = ((2*s+1)*wheight)/(2*nsig);
	    vmin[s] = 9999; vmax[s] = -9999;
	}
	tbmid = (nsig==1)?(int)(0.75*wheight):(base[nsig/2-1]+base[nsig/2])/2;
	
	/* Allocate the signal buffer. */
	trlen = (long)(wwsec * sps);
	if (xppsi < 0.75) {
	    compress = 1;
	    while ((v=malloc((unsigned)(1.5*wwidth*nsig*sizeof(int))))==NULL)
		wwidth /= 2;
	}
	else while ((v = malloc((unsigned)(trlen*nsig*sizeof(int)))) == NULL)
	    trlen -= (long)(sps/5.);
	
	/* Skip to the starting time. */
	isigsettime(t0);
	
	if (compress) {
	    static int x0, vv0[WFDB_MAXSIG], vmn[WFDB_MAXSIG],
	               vmx[WFDB_MAXSIG], vv[WFDB_MAXSIG];
	    
	    getvec(vp = v);
	    for (s = 0; s < nsig; s++)
		vmax[s] = vmin[s] = vmn[s] = vmx[s] = vv0[s] = vp[s];
	    for (i = 1, x0 = 0; i < trlen && getvec(vv) > 0; i++) {
		int xx;
		if ((xx = (int)(i*xppsi)) > x0) {
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
	
	SelectObject(hDC, signal_pen);
	for (s = 0; s < nsig; s++) {
	    yb = base[s];
	    v0 = (vmax[s] + vmin[s])/2;
	    ysc = yppadu[s];
	    if (nflag)
                TextOut(hDC, xtr(1.), (int)(yb - yppmm*10), si[s].desc,
			strlen(si[s].desc));
	    vp = v+s;
	    MoveTo(hDC, 0, yt(*vp - v0));
	    vp += nsig;
	    if (compress)
		for (j = 1; j < i; j++, vp += nsig)
		    LineTo(hDC, j, yt(*vp - v0));
	    else
		for (j = 1; j < i; j++, vp += nsig)
		    LineTo(hDC, xt(j), yt(*vp - v0));
	    vmax[s] = -9999; vmin[s] = 9999;
	}
	
	/* Free the signal buffer.  (This is not strictly necessary for a large
	   memory model Windows program, since the version of malloc used in
	   this case allocates memory from the automatic data segment.) */
	free(v);
    }
    
    if (nann > 0) {     /* Show annotations. */
	WFDB_Annotation annot;
	
	iannsettime(t0);
	if (t0 == 0L) {         /* skip modification labels */
	    int i;
	    
	    while ((i = getann(0, &annot)) == 0 &&
		   annot.time == 0L && annot.anntyp == NOTE)
		;
	    if (i == 0) ungetann(0, &annot);
	}
	if (mflag) SelectObject(hDC, marker_pen);
	SetTextColor(hDC, MARKER_COLOR);
	tbhigh = tbmid - cheight;
	tblow = tbmid + cheight;
	do {
	    if (getann(0, &annot) < 0) break;
	    switch (annot.anntyp) {
	      case RHYTHM:
		y = tblow;
		if (annot.aux && !show_aux)
		    p = annot.aux+1;
		else
		    p = annstr(annot.anntyp);
		break;
	      case STCH:
	      case TCH:
	      case NOTE:
		y = tbhigh;
		if (annot.aux && !show_aux)
		    p = annot.aux+1;
		else
		    p = annstr(annot.anntyp);
		break;
              case LINK:
                y = tbhigh;
                if (annot.aux && !show_aux) {
                    char *p1 = annot.aux+1, *p2 = p1 + *(p1-1);
                    p = p1;
                    while (p1 < p2) {
                        if (*p1 == ' ' || *p1 == '\t') {
                             p = p1 + 1;
                             break;
                        }
                        p1++;
                    }
                    SetTextColor(hDC, SIGNAL_COLOR);
                }
                break;
	      case NOISE:
		y = tbhigh;
		if (annot.subtyp == -1) { p = "U"; break; }
		/* The existing scheme is good for up to 4 signals;  it can
		   be easily extended to 8 or 12 signals using the chan and
		   num fields, or to an arbitrary number of signals using
		   `aux'. */
		for (s = 0; s < nsig && s < 4; s++) {
		    if (annot.subtyp & (0x10 << s))
			ts[s] = 'u';    /* signal s is unreadable */
		    else if (annot.subtyp & (0x01 << s))
			ts[s] = 'n';    /* signal s is noisy */
		    else
			ts[s] = 'c';    /* signal s is clean */
		}
		ts[s] = '\0';
		p = ts; break;
	      default:
		y = tbmid; p = annstr(annot.anntyp); break;
	    }
	    x = (int)((annot.time - t0) * xppsi);
	    if (mflag) {
		MoveTo(hDC, x, 0);
		LineTo(hDC, x, y - tm.tmExternalLeading);
	    }
	    TextOut(hDC, x, y, p, strlen(p));
	    if (show_subtyp) {
		y += cheight;
		sprintf(ts, "%d", annot.subtyp);
		TextOut(hDC, x, y, ts, strlen(ts));
	    }
	    if (show_chan) {
		y += cheight;
		sprintf(ts, "%d", annot.chan);
		TextOut(hDC, x, y, ts, strlen(ts));
	    }
	    if (show_num) {
		y += cheight;
		sprintf(ts, "%d", annot.num);
		TextOut(hDC, x, y, ts, strlen(ts));
	    }
	    if (show_aux && annot.aux) {
		y += cheight;
		p = annot.aux+1;
		TextOut(hDC, x, y, p, strlen(p));
	    }
	    if (mflag) {
		MoveTo(hDC, x, y + cheight);
		LineTo(hDC, x, wheight);
	    }
            if (annot.anntyp == LINK)
                SetTextColor(hDC, MARKER_COLOR);
	} while (annot.time < t1);
    }
    SelectObject(hDC, hOldPen);
    EndPaint (hWnd,  &ps);
}

void print_window(hWnd, printer)
HWND hWnd;
char *printer;
{
    char *p, ts[80], *jobname = "WVIEW";
    double pwsec, phmv, pxppmm, pyppmm, pxppsi, pyppadu[WFDB_MAXSIG], pysc;
    DWORD dwextent;
    HDC hDC;
    HPEN hOldPen;
    int base[WFDB_MAXSIG], compress = 0, j, s, slen, tbhigh, tblow, tbmid,
        *v, *vp, vmax[WFDB_MAXSIG], vmin[WFDB_MAXSIG], v0, x, y, yb, pwidth,
        pheight, pcheight;
    long i, trlen;
    TEXTMETRIC ptm;
    
    /* Set up a device context for printing. */
    if ((hDC = GetPrinterDC(printer)) == NULL) return;
    if (Escape(hDC, STARTDOC, strlen(jobname), jobname, NULL) <= 0) {
	DeleteDC(hDC);
	return;
    }
    /* Determine the printer resolution and paper size in sec and mV. */
    pwidth = GetDeviceCaps(hDC,HORZRES);
    pheight = GetDeviceCaps(hDC,VERTRES);
    pxppmm = pwidth/(double)GetDeviceCaps(hDC,HORZSIZE);
    pyppmm = pheight/(double)GetDeviceCaps(hDC,VERTSIZE);
    pwsec = pwidth/(ptscale*pxppmm);
    phmv = pheight/(pvscale*pyppmm);
    
    GetTextMetrics(hDC, &ptm);
    pcheight = ptm.tmExternalLeading + ptm.tmHeight;
    
    /* Calculate the time scale in pixels per sample interval. */
    pxppsi = pxppmm * ptscale / sps;
    
    /* Draw in transparent mode (do not clear spaces around letters,
       etc., to white). */
    SetBkMode(hDC, TRANSPARENT);
    
    /* Draw the grid, if requested. */
    hOldPen = SelectObject(hDC, pgrid_pen);
    if (pgtflag) {
	double gdx = pxppmm*ptscale/GTICKSPERSEC;
	int i;
	
	for (i = 1, x = (int)gdx; x < pwidth; i++, x = (int)(i*gdx)) {
	    MoveTo(hDC, x, 0);
	    LineTo(hDC, x, pheight - 1);
	}
    }
    if (pgvflag) {
	double gdy = pyppmm*pvscale/GTICKSPERMV;
	int i;
	
	for (i = 1, y = (int)gdy; y < pheight; i++, y = (int)(i*gdy)){
	    MoveTo(hDC, 0, y);
	    LineTo(hDC, pwidth - 1, y);
	}
    }
    
    if (nsig > 0 || nann > 0) {
	/* Set color for time stamps and signal names. */
	SetTextColor(hDC, SIGNAL_COLOR);
	
	/* Show the starting time in the lower left corner. */
        slen = sprintf(ts, "%s", timstr(ptmode ? -pt0 : pt0));
	for (p = ts; *p == ' '; p++, slen--)
	    ;   /* discard initial spaces, if any */
	x = pxtr(2.);
	y = pytr(7.5) - ptm.tmExternalLeading;
	TextOut(hDC, x, y, p, slen);
	
	/* Determine the ending time. */
	pt1 = pt0 + (WFDB_Time)(sps*(int)pwsec);
	
	/* Show the ending time in the lower right corner. */
        slen = sprintf(ts, "%s", timstr(ptmode ? -pt1 : pt1));
	for (p = ts; *p == ' '; p++, slen--)
	    ;   /* discard initial spaces, if any */
	dwextent = GetTextExtent(hDC, p, slen);
	x = pxt(sps*(int)pwsec) - LOWORD(dwextent) - pxtr(2.);
	TextOut(hDC, x, y, p, slen);
    }
    
    /* Show the record and annotator names centered at the top. */
    if (*record && nann > 0) 
	slen = sprintf(ts, "Record %s, Annotator %s", record, annotator);
    else if (*record)
	slen = sprintf(ts, "Record %s", record);
    else
	slen = 0;
    if (slen > 0) {
	dwextent = GetTextExtent(hDC, ts, slen);
	x = pxt(sps*0.5*(int)pwsec) - LOWORD(dwextent)/2;
	TextOut(hDC, x, pcheight, ts, slen);
    }
    
    /* Show scales centered at the bottom. */
    slen = sprintf(ts, "%s  %s", tst[ptsi], vst[pvsi]);
    dwextent = GetTextExtent(hDC, ts, slen);
    x = pxt(sps*0.5*(int)pwsec) - LOWORD(dwextent)/2;
    TextOut(hDC, x, y - pcheight, ts, slen);
    
    /* Show the copyright notice centered at the bottom. */
    slen = sprintf(ts, "Printed by WVIEW %s  Copyright \251 MIT 1997",
		   VERSION);
    dwextent = GetTextExtent(hDC, ts, slen);
    x = pxt(sps*0.5*(int)pwsec) - LOWORD(dwextent)/2;
    TextOut(hDC, x, y, ts, slen);
    
    if (nsig < 1)
	tbmid = pheight/2;      /* Set standard ordinate for annotations. */
    else {      /* Draw signals. */
	/* Determine signal amplitude scales and baselines. */
	for (s = 0; s < nsig; s++) {
	    pyppadu[s] = (pyppmm * pvscale) / sg[s];
	    base[s] = ((2*s+1)*pheight)/(2*nsig);
	    vmin[s] = 9999; vmax[s] = -9999;
	}
	tbmid = (nsig==1)?(int)(0.75*pheight):(base[nsig/2-1]+base[nsig/2])/2;
	
	/* Allocate the signal buffer. */
	trlen = (long)(pwsec * sps);
	if (pxppsi < 0.75) {
	    compress = 1;
	    while ((v=malloc((unsigned)(1.5*pwidth*nsig*sizeof(int))))==NULL)
		pwidth /= 2;
	}
	else while ((v = malloc((unsigned)(trlen*nsig*sizeof(int)))) == NULL)
	    trlen -= (long)(sps/5.);
	
	/* Skip to the starting time. */
	isigsettime(pt0);
	
	if (compress) {
	    static int x0, vv0[WFDB_MAXSIG], vmn[WFDB_MAXSIG],
	               vmx[WFDB_MAXSIG], vv[WFDB_MAXSIG];
	    
	    getvec(vp = v);
	    for (s = 0; s < nsig; s++)
		vmax[s] = vmin[s] = vmn[s] = vmx[s] = vv0[s] = vp[s];
	    for (i = 1, x0 = 0; i < trlen && getvec(vv) > 0; i++) {
		int xx;
		if ((xx = (int)(i*pxppsi)) > x0) {
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
	
	SelectObject(hDC, signal_pen);
	for (s = 0; s < nsig; s++) {
	    yb = base[s];
	    v0 = (vmax[s] + vmin[s])/2;
	    pysc = pyppadu[s];
	    if (nflag)
		TextOut(hDC, pxtr(1.), (int)(yb + pyppmm*2), si[s].desc,
			strlen(si[s].desc));
	    vp = v+s;
	    MoveTo(hDC, 0, pyt(*vp - v0));
	    vp += nsig;
	    if (compress)
		for (j = 1; j < i; j++, vp += nsig)
		    LineTo(hDC, j, pyt(*vp - v0));
	    else
		for (j = 1; j < i; j++, vp += nsig)
		    LineTo(hDC, pxt(j), pyt(*vp - v0));
	    vmax[s] = -9999; vmin[s] = 9999;
	}
	
	/* Free the signal buffer.  (This is not strictly necessary for a large
	   memory model Windows program, since the version of malloc used in
	   this case allocates memory from the automatic data segment.) */
	free(v);
    }
    
    if (nann > 0) {     /* Show annotations. */
	WFDB_Annotation annot;
	
	iannsettime(pt0);
	if (pt0 == 0L) {                /* skip modification labels */
	    int i;
	    
	    while ((i = getann(0, &annot)) == 0 &&
		   annot.time == 0L && annot.anntyp == NOTE)
		;
	    if (i == 0) ungetann(0, &annot);
	}
	if (pmflag) SelectObject(hDC, marker_pen);
	SetTextColor(hDC, MARKER_COLOR);
	tbhigh = tbmid - pcheight;
	tblow = tbmid + pcheight;
	do {
	    if (getann(0, &annot) < 0) break;
	    switch (annot.anntyp) {
	      case RHYTHM:
		y = tblow;
		if (annot.aux && !pshow_aux)
		    p = annot.aux+1;
		else
		    p = annstr(annot.anntyp);
		break;
	      case STCH:
	      case TCH:
	      case NOTE:
		y = tbhigh;
		if (annot.aux && !pshow_aux)
		    p = annot.aux+1;
		else
		    p = annstr(annot.anntyp);
		break;
	      case NOISE:
		y = tbhigh;
		if (annot.subtyp == -1) { p = "U"; break; }
		/* The existing scheme is good for up to 4 signals;  it can
		   be easily extended to 8 or 12 signals using the chan and
		   num fields, or to an arbitrary number of signals using
		   `aux'. */
		for (s = 0; s < nsig && s < 4; s++) {
		    if (annot.subtyp & (0x10 << s))
			ts[s] = 'u';    /* signal s is unreadable */
		    else if (annot.subtyp & (0x01 << s))
			ts[s] = 'n';    /* signal s is noisy */
		    else
			ts[s] = 'c';    /* signal s is clean */
		}
		ts[s] = '\0';
		p = ts; break;
	      default:
		y = tbmid; p = annstr(annot.anntyp); break;
	    }
	    x = (int)((annot.time - pt0) * pxppsi);
	    if (pmflag) {
		MoveTo(hDC, x, 0);
		LineTo(hDC, x, y - ptm.tmExternalLeading);
	    }
	    TextOut(hDC, x, y, p, strlen(p));
	    if (pshow_subtyp) {
		y += pcheight;
		sprintf(ts, "%d", annot.subtyp);
		TextOut(hDC, x, y, ts, strlen(ts));
	    }
	    if (pshow_chan) {
		y += cheight;
		sprintf(ts, "%d", annot.chan);
		TextOut(hDC, x, y, ts, strlen(ts));
	    }
	    if (pshow_num) {
		y += pcheight;
		sprintf(ts, "%d", annot.num);
		TextOut(hDC, x, y, ts, strlen(ts));
	    }
	    if (pshow_aux && annot.aux) {
		y += pcheight;
		p = annot.aux+1;
		TextOut(hDC, x, y, p, strlen(p));
	    }
	    if (pmflag) {
		MoveTo(hDC, x, y + pcheight);
		LineTo(hDC, x, pheight);
	    }
	} while (annot.time < pt1);
    }
    SelectObject(hDC, hOldPen);
    if (Escape(hDC, NEWFRAME, 0, NULL, NULL) > 0)
	Escape(hDC, ENDDOC, 0, NULL, NULL);
    DeleteDC(hDC);
}
