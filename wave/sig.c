/* file: sig.c		G. Moody	27 April 1990
			Last revised:	29 April 1999
Signal display functions for WAVE

-------------------------------------------------------------------------------
WAVE: Waveform analyzer, viewer, and editor
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

#include "wave.h"
#include "xvwave.h"

static void show_signal_names(), show_signal_baselines();

/* Show_display_list() plots the display list pointed to by its argument. */

static struct display_list *lp_current;
static int highlighted = -1;

int in_siglist(i)
int i;
{
    int nsl;

    for (nsl = 0; nsl < siglistlen; nsl++)
	if (siglist[nsl] == i) return (1);
    return (0);
}

static void show_display_list(lp)
struct display_list *lp;
{
    int i, in_siglist();

    lp_current = lp;
    if (!lp) return;
    if (sig_mode == 0)
	for (i = 0; i < nsig; i++) {
	    if (lp->vlist[i]) {
		lp->vlist[i][0].y += base[i];
		XDrawLines(display, xid, draw_sig, lp->vlist[i], lp->ndpts,
		       CoordModePrevious);
		lp->vlist[i][0].y -= base[i];
	    }
	}
    else
	for (i = 0; i < siglistlen; i++) {
	    if (lp->vlist[siglist[i]]) {
		lp->vlist[siglist[i]][0].y += base[i];
		XDrawLines(display, xid, draw_sig, lp->vlist[siglist[i]],
			   lp->ndpts, CoordModePrevious);
		lp->vlist[siglist[i]][0].y -= base[i];
	    }
	}
    highlighted = -1;
}

void sig_highlight(i)
int i;
{
    if (!lp_current) return;
    if (0 <= highlighted && highlighted < lp_current->nsig) {
	if (sig_mode == 0) {
	    lp_current->vlist[highlighted][0].y += base[highlighted];
	    XDrawLines(display, xid, unhighlight_sig,
		       lp_current->vlist[highlighted],
		       lp_current->ndpts, CoordModePrevious);
	    XDrawLines(display, xid, draw_sig,
		       lp_current->vlist[highlighted],
		       lp_current->ndpts, CoordModePrevious);
	    lp_current->vlist[highlighted][0].y -= base[highlighted];
	}
	else {
	    int j;
	    for (j = 0; j < siglistlen; j++) {
		if (siglist[j] == highlighted) {
		    lp_current->vlist[highlighted][0].y += base[j];
		    XDrawLines(display, xid, unhighlight_sig,
			       lp_current->vlist[highlighted],
			       lp_current->ndpts, CoordModePrevious);
		    XDrawLines(display, xid, draw_sig,
			       lp_current->vlist[highlighted],
			       lp_current->ndpts, CoordModePrevious);
		    lp_current->vlist[highlighted][0].y -= base[j];
		}
	    }
	}
    }
    highlighted = i;
    if (0 <= highlighted && highlighted < lp_current->nsig) {
	if (sig_mode == 0) {
	    lp_current->vlist[highlighted][0].y += base[highlighted];
	    XDrawLines(display, xid, clear_sig,
		       lp_current->vlist[highlighted],
		       lp_current->ndpts, CoordModePrevious);
	    XDrawLines(display, xid, highlight_sig,
		       lp_current->vlist[highlighted],
		       lp_current->ndpts, CoordModePrevious);
	    lp_current->vlist[highlighted][0].y -= base[highlighted];
	}
	else {
	    int j;
	    for (j = 0; j < siglistlen; j++) {
		if (siglist[j] == highlighted) {
		    lp_current->vlist[highlighted][0].y += base[j];
		    XDrawLines(display, xid, clear_sig,
			       lp_current->vlist[highlighted],
			       lp_current->ndpts, CoordModePrevious);
		    XDrawLines(display, xid, highlight_sig,
			       lp_current->vlist[highlighted],
			       lp_current->ndpts, CoordModePrevious);
		    lp_current->vlist[highlighted][0].y -= base[j];
		}
	    }
	}
    }
}

/* Do_disp() executes a display request.  The display will show nsamp samples
of nsig signals, starting at display_start_time. */

void do_disp()
{
    char *tp;
    int c, i, x0, x1, y0;
    struct display_list *lp;

    /* This might take a while ... */
    xv_set(frame, FRAME_BUSY, TRUE, NULL);

    /* Show the grid if requested. */
    show_grid();

    /* Make sure that the requested time is reasonable. */
    if (display_start_time < 0) display_start_time = 0;

    /* Update the panel items that indicate the start and end times. */
    tp = wtimstr(display_start_time);
    set_start_time(tp);
    while (*tp == ' ') tp++;
    y0 = canvas_height - mmy(2);
    x0 = mmx(2);
    XDrawString(display, xid, time_mode == 1 ? draw_ann : draw_sig,
		x0, y0, tp, strlen(tp));
    tp = wtimstr(display_start_time + nsamp);
    set_end_time(tp);
    while (*tp == ' ') tp++;
    x1 = canvas_width - XTextWidth(font, tp, strlen(tp)) - mmx(2);
    XDrawString(display, xid, time_mode == 1 ? draw_ann : draw_sig,
		x1, y0, tp, strlen(tp));

    /* Show the annotations, if available. */
    show_annotations(display_start_time, nsamp);

    /* If requested, show the signal names. */
    if (show_signame) show_signal_names();

    /* Get a display list for the requested screen, and show it. */
    lp = find_display_list(display_start_time);
    show_display_list(lp);

    /* If requested, show the signal baselines. */
    if (show_baseline) show_signal_baselines(lp);

    xv_set(frame, FRAME_BUSY, FALSE, NULL);
}

static int nlists;
static int vlist_size;

/* Get_display_list() obtains storage for display lists from the heap (via
malloc) or by recycling a display list in the cache.  Since the screen duration
(nsamp) does not change frequently, get_display_list() calculates the pixel
abscissas when a new display list is obtained from the heap, and recalculates
them only when nsamp has been changed. */

static struct display_list *get_display_list()
{
    int i, maxx, x;
    static int max_nlists = MAX_DISPLAY_LISTS;
    struct display_list *lp = NULL, *lpl;

    if (nsig > WFDB_MAXSIG) return (NULL);

    if (nlists < max_nlists) {
	lp = (struct display_list *)calloc(1, sizeof(struct display_list));
	if (lp) nlists++;
    }
    if (lp == NULL)
	switch (nlists) {
	    case 0: return (NULL);
	    case 1: lp = first_list; break;
	    default: lpl = first_list; lp = lpl->next;
		while (lp->next) {
		    lpl = lp;
		    lp = lp->next;
		}
		lpl->next = NULL;
		break;
	}

    if (canvas_width > vlist_size) vlist_size = canvas_width;

    for (i = 0; i < nsig; i++) {
	if (lp->vlist[i] == NULL) {
	    if ((lp->vlist[i] =
		 (XPoint *)calloc(vlist_size, sizeof(XPoint)))==NULL) {
		while (--i >= 0)
		    free(lp->vlist[i]);
		free(lp);
		max_nlists = --nlists;
		return (get_display_list());
	    }
	}
	
	/* If there are more samples to be shown than addressable x-pixels
	   in the window, the abscissas are simply the integers from 0 to
	   the canvas width (and some compression will be needed).  If the
	   largest abscissa is correct, all of the others must also be correct,
	   and they need not be reset. */
	if (nsamp > canvas_width) {
	    x = maxx = canvas_width - 1;
	    if (lp->xmax != x) {
		int xx;

		lp->vlist[i][0].x = 0;	/* absolute first abscissa */
		for (xx = 1; xx <= x; xx++)
		    lp->vlist[i][xx].x = 1;	/* relative to previous */
	    }
	}

	/* Otherwise, no compression is needed, and the abscissas must be
	   distributed across the window (at intervals > 1 pixel).  Again,
	   if the largest abscissa is correct, all of the others must also
	   be correct, and no computation is needed. */
	else {
	    x = maxx = nsamp - 1;
	    if (lp->xmax != x*tscale) {
		int xp, xpp;

		lp->vlist[i][0].x = xp = 0;    /* absolute first abscissa */
		for (x = 1; x < vlist_size; x++) {
		    xpp = xp;
		    xp = x*tscale;
		    lp->vlist[i][x].x = xp - xpp;	/* relative to prev */
		}
	    }
	}
    }
    lp->next = first_list;
    lp->nsig = nsig;
    lp->npoints = nsamp;
    lp->xmax = maxx;
    return (first_list = lp);
}

/* Find_display_list() obtains a display list beginning at the sample number
specified by its argument.  If such a list (with the correct duration) is
found in the cache, it can be returned immediately.  Otherwise, the function
reads the requested segment and determines the pixel ordinates of the
vertices of the polylines for each signal. */

struct display_list *find_display_list(fdl_time)
long fdl_time;
{
    int c, i, j, x, x0, y, ymax, ymin;
    int v[WFDB_MAXSIG], v0[WFDB_MAXSIG], vmax[WFDB_MAXSIG], vmin[WFDB_MAXSIG];
    struct display_list *lp;
    XPoint *tp;

    if (fdl_time < 0L) fdl_time = -fdl_time;
    /* If the requested display list is in the cache, return it at once. */
    for (lp = first_list; lp; lp = lp->next)
	if (lp->start == fdl_time && lp->npoints == nsamp) return (lp);

    /* Give up if a display list can't be allocated, or if we can't skip
       to the requested segment, or if we can't read at least one sample. */
    if ((lp = get_display_list()) == NULL ||
	(fdl_time != strtim("i") && isigsettime(fdl_time) < 0) ||
	getvec(v0) < 0)
	    return (NULL);

    /* Record the starting time. */
    lp->start = fdl_time;

    /* Set the starting point for each signal. */
    for (c = 0; c < nsig; c++) {
	vmin[c] = vmax[c] = v0[c];
	if (v0[c] == -32768)
	    lp->vlist[c][0].y = -32768;
	else
	    lp->vlist[c][0].y = v0[c]*vscale[c];
    }

    /* If there are more than canvas_width samples to be shown, compress the
       data. */
    if (nsamp > canvas_width) {
	for (i = 1, x0 = 0; i < nsamp && getvec(v) > 0; i++) {
	    if ((x = i*tscale) > x0) {
		x0 = x;
		for (c = 0; c < nsig; c++) {
		    if (v[c] > vmax[c]) vmax[c] = v[c];
		    else if (v[c] < vmin[c]) vmin[c] = v[c];
		    if (vmax[c] - v0[c] > v0[c] - vmin[c])
			v0[c] = vmin[c] = vmax[c];
		    else
			v0[c] = vmax[c] = vmin[c];
		    if (v0[c] == -32768)
			lp->vlist[c][x0].y = -32768;
		    else
			lp->vlist[c][x0].y = v0[c]*vscale[c];
		}
	    }
	    else {
		for (c = 0; c < nsig; c++) {
		    if (v[c] > vmax[c]) vmax[c] = v[c];
		    else if (v[c] < vmin[c]) vmin[c] = v[c];
		}
	    }
	}
	i = x0+1;
    }
    /* If there are canvas_width or fewer samples to be shown, no compression
       is necessary. */
    else
	for (i = 1; i < nsamp && getvec(v) > 0; i++)
	    for (c = 0; c < nsig; c++) {
		if (v[c] == -32768)
		    lp->vlist[c][i].y = -32768;
		else
		    lp->vlist[c][i].y = v[c]*vscale[c];
	    }

    /* Record the number of displayed points.  This may be less than
       expected at the end of the record. */
    lp->ndpts = i;

    /* Set the y-offset so that the signal will be vertically centered about
       the nominal baseline if the midrange is near the mean.  The y-offset
       is actually a weighted sum of the midrange and the mean, which favors
       the mean if the two values differ significantly. */
    for (c = 0; c < nsig; c++) {
	int dy;		/* the y-offset */
	int n;		/* the number of valid ordinates */
	int ymid;	/* the midpoint of the ordinate range */
	long ymean;	/* the mean of the ordinates */
	double w;	/* weight assigned to ymean in y-offset calculation */

	tp = lp->vlist[c];
	ymean = ymax = ymin = tp[0].y;
	for (j = i-1, n = 1; j >= 0; j--) {
	    y = tp[j].y;
	    if (y > ymax) ymax = y;
	    else if (y < ymin && y != -32768) ymin = y;
	    if (y != -32768) { ymean += y; n++; }
	}
	ymid = (ymax + ymin)/2;
	ymean /= n;
	/* Since ymin <= ymid <= ymax, the next lines imply 0 <= w <= 1 */
	if (ymid > ymean) /* in this case, ymax must be > ymean */
	    w = (ymid - ymean)/(ymax - ymean);
	else if (ymid < ymean) /* in this case, ymin must be < ymin */
	    w = (ymean - ymid)/(ymean - ymin);
	else w = 1.0;
	dy = -(ymid + ((double)ymean-ymid)*w);
	for (j = i-1; j >= 0; j--) {
	    /* The bounds-checking below shouldn't be necessary (the X server
	       should clip at the canvas boundaries), but Sun's X11/NeWS
	       server will crash (and may bring the system down with it) if
	       run on a system with the GX accelerator and if passed
	       sufficiently out-of-bounds ordinates (maybe abscissas, too --
	       this hasn't been tested).  This bug is present in both the
	       original X11/NeWS server and the GFX revision.  It appears that
	       the bug can be avoided if the maximum distance from the window
	       edge to the out-of-bounds ordinate is less than about 2000
	       pixels (although this may be dependent on the window height).
	       This bug has not been observed with other X servers. */
	    if ((tp[j].y += dy) < -canvas_height) tp[j].y = -canvas_height;
	    else if  (tp[j].y > canvas_height) tp[j].y = canvas_height;
	    /* Convert all except the first ordinate to relative ordinates. */
	    if (j < i-1) tp[j+1].y -= tp[j].y;
	}
	if (dc_coupled[c]) lp->sb[c] = sigbase[c]*vscale[c] + dy;
    }	    
    return (lp);
}

/* Clear_cache() marks all of the display lists in the cache as invalid.  This
   function should be executed whenever the gain (vscale) or record is changed,
   or whenever the canvas width has been increased. */
void clear_cache()
{
    int i;
    struct display_list *lp;

    if (canvas_width > vlist_size) {
	for (lp = first_list; lp; lp = lp->next) {
	    for (i = 0; i < NSIG && lp->vlist[i] != NULL; i++) {
		free(lp->vlist[i]);
		lp->vlist[i] = NULL;
	    }
	}
	vlist_size = 0;
    }
    else {
	for (lp = first_list; lp; lp = lp->next) {
	    lp->start = -1;
	    lp->npoints = 0;
	}
    }
}

static void show_signal_names()
{
    int i, xoff, yoff;

    xoff = mmx(5);
    yoff = (nsig > 1) ? (base[1] - base[0])/3 : canvas_height/3;
    if (sig_mode == 0) 
	for (i = 0; i < nsig; i++)
	    XDrawString(display, xid, draw_sig, xoff, base[i] - yoff,
			signame[i], strlen(signame[i]));
    else
	for (i = 0; i < siglistlen; i++)
	    XDrawString(display, xid, draw_sig, xoff, base[i] - yoff,
			signame[siglist[i]], strlen(signame[siglist[i]]));
}

static void show_signal_baselines(lp)
struct display_list *lp;
{
    int i, l, xoff, yoff;

    yoff = mmy(2);
    for (i = 0; i < nsig; i++) {
	if (dc_coupled[i] && 0 <= lp->sb[i] && lp->sb[i] < canvas_height) {
	    XDrawLine(display, xid, draw_ann,
		      0, lp->sb[i]+base[i], canvas_width, lp->sb[i]+base[i]);
	    if (blabel[i]) {
		l = strlen(blabel[i]);
		xoff = canvas_width - XTextWidth(font, blabel[i], l) - mmx(2);
		XDrawString(display, xid, draw_sig,
			    xoff, lp->sb[i]+base[i] - yoff, blabel[i], l);
	    }
	}
    }
}

/* Return window y-coordinate corresponding to the level of displayed trace
   i at abscissa x. */
int sigy(i, x)
int i, x;
{
    int ix, j = -1, xx, yy;

    if (sig_mode == 0) j = i;
    else if (0 <= i && i < siglistlen) j = siglist[i];
    if (j < 0 || j >= nsig || lp_current->vlist[j] == NULL) return (-1);
    if (nsamp > canvas_width) ix = x;
    else ix = (int)x/tscale;
    if (ix >= lp_current->ndpts) ix = lp_current->ndpts - 1;
    for (xx = yy = 0; xx < ix; xx++)
	yy += lp_current->vlist[j][xx].y;
    return (yy + base[i]);
}
