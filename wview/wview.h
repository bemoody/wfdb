/* file: wview.h	G. Moody	20 January 1993
                        Last revised:      7 May 1999

-------------------------------------------------------------------------------
wview.h: Constants for wview
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

/* wview version string */
#define VERSION "Version 1.04 alpha"

/* The following set of topic definitions must match those in the [MAP] section
   of wview.hpj -- keep them in agreement! */
#define Choosing_Topic	101
#define Browsing_Topic	102
#define Options_Topic	103
#define Searching_Topic	104
#define Printing_Topic	105

#define IDM_NEW		1
#define IDM_OPEN	2
#define IDM_SAVE	3
#define IDM_SAVEAS	4
#define IDM_PRINT	5
#define IDM_EXIT	6

#define IDM_UNDO       10
#define IDM_CUT	       11
#define IDM_COPY       12
#define IDM_PASTE      13
#define IDM_DEL	       14
#define IDM_FIND       15

#define IDM_VIEW       20
#define IDM_PROPT      21

#define IDM_HELP_INDEX		41
#define IDM_HELP_CHOOSING	42
#define IDM_HELP_BROWSING	43
#define IDM_HELP_OPTIONS	44
#define IDM_HELP_SEARCHING	45
#define IDM_HELP_PRINTING	46
#define IDM_ABOUT		47

#define IDD_RECORD    102
#define IDD_ANNOTATOR 104
#define IDD_WFDBPATH    106
#define IDD_WFDBCAL     108

#define IDD_FINDN     201
#define IDD_FINDP     202
#define IDD_RECENTER  203
#define IDD_FNTYPE    204
#define IDD_FPTYPE    205
#define IDD_RCTIME    206

#define IDD_PPRLABEL  300
#define IDD_PPRINTER  301
#define IDD_PRANGE    302
#define IDD_PRREC     303
#define IDD_PRWIN     304
#define IDD_PRSEG     305
#define IDD_PRFLABEL  306
#define IDD_PRFROM    307
#define IDD_PRTLABEL  308
#define IDD_PRTO      309
#define IDD_PFILE     310
#define IDD_PCLABEL   311
#define IDD_PCOPIES   312
#define IDD_PCOLLATE  313
#define IDD_PSETUP    314
#define IDD_POPTIONS  315

#define IDD_PODEF     401
#define IDD_POSCALES  402
#define IDD_POSTLABEL 403
#define IDD_POSTIME   404
#define IDD_POSALABEL 405
#define IDD_POSAMP    406
#define IDD_POGRID    407
#define IDD_POGTIME   408
#define IDD_POGAMP    409
#define IDD_POANN     410
#define IDD_POAMARK   411
#define IDD_POASUB    412
#define IDD_POACHAN   413
#define IDD_POANUM    414
#define IDD_POAAUX    415
#define IDD_POOTHER   416
#define IDD_POOSB     417
#define IDD_POOSN     418
#define IDD_POOCLABEL 419
#define IDD_POOCOM    420
#define IDD_POCALIB   421
#define IDD_POHELP    422
#define IDD_POTMODE   423

#define IDD_VODEF     501
#define IDD_VOSCALES  502
#define IDD_VOSTLABEL 503
#define IDD_VOSTIME   504
#define IDD_VOSALABEL 505
#define IDD_VOSAMP    506
#define IDD_VOGRID    507
#define IDD_VOGTIME   508
#define IDD_VOGAMP    509
#define IDD_VOANN     510
#define IDD_VOAMARK   511
#define IDD_VOASUB    512
#define IDD_VOACHAN   513
#define IDD_VOANUM    514
#define IDD_VOAAUX    515
#define IDD_VOOTHER   516
#define IDD_VOOSB     517
#define IDD_VOOSN     518
#define IDD_VOCALIB   521
#define IDD_VOHELP    522
#define IDD_VOTMODE   523

int PASCAL WinMain(HANDLE, HANDLE, LPSTR, int);
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
long FAR PASCAL MainWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL FAR PASCAL About(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL Choose(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL Find(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL Print(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL PrintOptions(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL ViewOptions(HWND, unsigned, WORD, LONG);
