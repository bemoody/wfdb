/* file: vsetup.c       G. Moody        July 1989
			Last revised:  4 May 1999

-------------------------------------------------------------------------------
vsetup: Initialization for `view'
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
#include <stdlib.h>
#include <conio.h>
#include <graph.h>
#include <bios.h>

/* Keyboard scan codes */
#define UP      72
#define DOWN    80
#define LEFT    75
#define RIGHT   77
#define ENTER   28

/* Abbreviate incredibly long Microsoft function names. */
#define st(A,B) _settextposition(A,B)
#define sv(A)   _setvideomode(A)
#define dotted  _setlinestyle(0x5555)
#define solid   _setlinestyle(0xffff)
#define li(A,B) _putimage(A,B,b,_GXOR)

main(argc, argv)
int argc;
char *argv[];
{
    struct videoconfig vc;
    char far *b;
    FILE *ofile = NULL;
    int done, mode = 0, r0, left, right, top, bottom, width, height;
    unsigned int s1, s2;
    char buf[10];

    /* Take a command-line argument as a video mode number to be tried
       first;  if the argument is `-s', however, append the derived command 
       for setting VIEWP to the file named in the next argument. */
    if (argc > 1) {
	if (strcmp(argv[1], "-s") == 0 && argc > 2) {
	    if ((ofile = fopen(argv[2], "a")) == NULL) {
		fprintf(stderr, "can't append to `%s'\n", argv[2]);
		exit(1);
	    }
	}
	else switch (mode = atoi(argv[1])) {
	  case _MRES256COLOR:
	  case _VRES16COLOR:
	  case _VRES2COLOR:
	  case _ERESCOLOR:
	  case _HERCMONO:
	  case _ERESNOCOLOR:
	  case _HRES16COLOR:
	  case _MRES16COLOR:
	  case _MRES4COLOR:
	  case _HRESBW:
	  case _MRESNOCOLOR:
#if (_MSC_VER >= 700)
	  case _ORESCOLOR:
	  case _ORES256COLOR:
	  case _VRES256COLOR:
	  case _SRES16COLOR:
	  case _SRES256COLOR:
	  case _XRES16COLOR:
	  case _XRES256COLOR:
	  case _ZRES16COLOR:
	  case _ZRES256COLOR:
#endif
	    if (sv(mode)) break;
	    /* If unsucessful, fall through to the default case. */
	  default:
	    printf("Sorry, video mode %d cannot be used.\n", mode);
	    printf("Press ENTER and this program will try to find\n");
	    printf("a usable video mode: ");
	    fgets(buf, 10, stdin);
	    mode = 0;
	}
    }
    else mode = 0;

    /* Try to find a mode that works. */
    if (mode == 0 && !(mode = set_mode())) {
	printf("Your current video mode is not supported.\n");
	printf("Try typing `mode co80' before continuing.\n");
	exit(1);
    }

    /* Read the selected video mode configuration. */
    _getvideoconfig(&vc);

    /* Set the assumed display area based on the configuration structure.
       Note that the origin is at *top* left, and the coordinates increase
       going right or down.  Note also that the largest legal coordinates
       are one less than the number of pixels in each direction (since row
       and column 0 are legal). */
    left = top = 0;
    right = vc.numxpixels-1;
    bottom = vc.numypixels-1;

    /* Allocate a buffer for storage of the dotted lines which will be moved
       about when finding the displayable area. */
    s1 = _imagesize(left, top, left, bottom);
    s2 = _imagesize(left, top, right, top);
    b = (char far *)malloc((s1 > s2) ? s1 : s2);

    /* Messages will be placed near the middle of the screen.  We must be
       careful not to go beyond 40 columns per line, or we'll have problems
       with CGA displays.  It also seems as if we have to set the row and
       column at the beginning of each line, or we get double-spaced text
       in 320x200 CGA mode and single-spaced text in 640x200 CGA mode, and
       who knows what else.  Arghhh! */
    r0 = vc.numtextrows/2-2;

    /* Find the leftmost displayable abscissa. */
    st(r0,   1); printf("<- A vertical line should be visible   \n");
    st(r0+1, 1); printf("   to the left;  if not, press the     \n");
    st(r0+2, 1); printf("   right arrow key until it is visible.\n");
    st(r0+3, 1); printf("   To continue, press ENTER.");
    dotted; _moveto(left, top); _lineto(left, bottom); solid;
    _getimage(0, 0, 0, bottom, b);
    for (done = 0; !done; ) {
	switch ((_bios_keybrd(_KEYBRD_READ) & 0xff00) >> 8) {
	  case LEFT:
	    if (0 < left) { li(left--, top); li(left, top); } break;
	  case RIGHT:
	    if (left < right) { li(left++, top); li(left, top); } break;
	  case ENTER:
	    _moveto(left,top); _lineto(left,bottom); done = 1; break;
	}
    }

    /* Find the rightmost displayable abscissa. */
    st(r0,   1); printf("   A vertical line should be visible ->\n");
    st(r0+1, 1); printf("   to the right;  if not, press the    \n");
    st(r0+2, 1); printf("   left arrow key until it is visible. \n");
    st(r0+3, 1); printf("   To continue, press ENTER.");
    _moveto(left, top); _lineto(left, bottom); li(right, top);
    for (done = 0; !done; ) {
	switch ((_bios_keybrd(_KEYBRD_READ) & 0xff00) >> 8) {
	  case LEFT:
	    if (left < right) { li(right--, top); li(right, top); } break;
	  case RIGHT:
	    if (right < vc.numxpixels-1) {
		li(right++, top); li(right, top); } break;
	  case ENTER:
	    _moveto(right, top); _lineto(right, bottom); done = 1; break;
	}
    }

    /* Find the top displayable ordinate. */
    st(r0,   1); printf("   A horizontal line should be visible \n");
    st(r0+1, 1); printf("   above;  if not, press the           \n");
    st(r0+2, 1); printf("   down arrow key until it is visible. \n");
    st(r0+3, 1); printf("   To continue, press ENTER.");
    _moveto(left, top); _lineto(left, bottom);
    dotted; _moveto(left, top); _lineto(right, top); solid;
    _getimage(left, 0, right, 0, b);
    for (done = 0; !done; ) {
	switch ((_bios_keybrd(_KEYBRD_READ) & 0xff00) >> 8) {
	  case UP:
	    if (0 < top) { li(left, top--); li(left, top); } break;
	  case DOWN:
	    if (top < bottom) { li(left, top++); li(left, top); } break;
	  case ENTER:
	    _moveto(left, top); _lineto(right, top); done = 1; break;
	}
    }

    /* Find the bottom displayable ordinate. */
    st(r0,   1); printf("   A horizontal line should be visible \n");
    st(r0+1, 1); printf("   below;  if not, press the           \n");
    st(r0+2, 1); printf("   up arrow key until it is visible.   \n");
    st(r0+3, 1); printf("   To continue, press ENTER.");
    _moveto(left, top); _lineto(left, bottom); li(left, bottom);
    for (done = 0; !done; ) {
	switch ((_bios_keybrd(_KEYBRD_READ) & 0xff00) >> 8) {
	  case UP:
	    if (0 < bottom) { li(left, bottom--); li(left, bottom); } break;
	  case DOWN:
	    if (bottom < vc.numypixels-1) {
		li(left, bottom++); li(left, bottom); } break;
	  case ENTER:
	    _moveto(left, bottom); _lineto(right, bottom); done = 1; break;
	}
    }

    st(r0,   1); printf("   Please measure the length of one of \n");
    st(r0+1, 1); printf("   the horizontal lines in millimeters \n");
    st(r0+2, 1); printf("   and enter it here: ___              \n");
    st(r0+3, 1); printf("   To continue, press ENTER.\n");
    _moveto(left, top); _lineto(left, bottom); 
    st(r0+2, 23); scanf("%d", &width);

    st(r0,   1); printf("   Please measure the length of one of \n");
    st(r0+1, 1); printf("   the vertical lines in millimeters   \n");
    st(r0+2, 1); printf("   and enter it here: ___              \n");
    st(r0+3, 1); printf("   To continue, press ENTER.\n");
    _moveto(left, top); _lineto(left, bottom); 
    st(r0+2, 23); scanf("%d", &height);

    sv(_DEFAULTMODE);

    if (ofile == NULL) {
	printf("To use `view', either type the command below or put it in\n");
	printf("your `autoexec.bat' file (if you do the latter, it will be\n");
	printf("effective only after a reboot):\n\n");
	printf("    set VIEWP=%d,%d,%d,%d,%d,%d,%d\n",
		   mode, left, right, top, bottom, width, height);
    }    
    else {   
	fprintf(ofile, "set VIEWP=%d,%d,%d,%d,%d,%d,%d\n",
		   mode, left, right, top, bottom, width, height);
	fclose(ofile);    
    }
    exit(0);
}

int set_mode()
{
    if (sv(_VRES16COLOR)) return (_VRES16COLOR);        /* 640x480x4 */
    if (sv(_VRES2COLOR)) return (_VRES2COLOR);          /* 640x480x1 */
    if (sv(_ERESCOLOR)) return (_ERESCOLOR);            /* 640x350x2/4 */
    if (sv(_HERCMONO)) return (_HERCMONO);              /* 720x348x1 */
    if (sv(_ERESNOCOLOR)) return (_ERESNOCOLOR);        /* 640x350x1 */
    if (sv(_HRES16COLOR)) return (_HRES16COLOR);        /* 640x200x4 */
    if (sv(_HRESBW)) return (_HRESBW);                  /* 640x200x1 */
    if (sv(_MRES16COLOR)) return (_MRES16COLOR);        /* 320x200x4 */
    if (sv(_MRES4COLOR)) return (_MRES4COLOR);          /* 320x200x2 */
    if (sv(_MRESNOCOLOR)) return (_MRESNOCOLOR);        /* 320x200x2 */
    return (0);
}
