/* file: init.c		G. Moody	1 May 1990
			Last revised: 29 April 1999
Initialization functions for WAVE

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
#include <xview/notice.h>

static struct WFDB_siginfo df[WFDB_MAXSIG];

/* Open up a new ECG record. */
int record_init(s)
char *s;
{
    char ts[RNLMAX+30];
    int i, rebuild_list, tl;

    /* Suppress error messages from the WFDB library. */
    wfdbquiet();

    /* Do nothing if the current annotation list has been edited and the
       changes can't be saved. */
    if (post_changes() == 0)
	return (0);

    /* Check to see if the signal list must be rebuilt. */
    rebuild_list = (siglistlen == 0) | strcmp(record, s);

    /* Save the name of the new record in local storage. */
    strncpy(record, s, RNLMAX);

    /* Reset the frame title. */
    set_frame_title();
    /* Open as many signals as possible. */
    nsig = isigopen(record, df, WFDB_MAXSIG);
    /* Get time resolution for annotations in sample intervals.  Except in
       WFDB_HIGHRES mode (selected using the -H option), the resolution is
       1 sample interval.  In WFDB_HIGHRES mode, when editing a multi-frequency
       record, the resolution for annotation times is 1 frame interval (i.e.,
       getspf() sample intervals). */
    atimeres = getspf();
    /* By convention, a zero or negative sampling frequency is interpreted as
       if the value were WFDB_DEFFREQ (from wfdb.h);  the units are samples per
       second per signal. */
    if (nsig < 0 || (freq = sampfreq(NULL)) <= 0.) freq = WFDB_DEFFREQ;

    /* Quit unless at least one signal can be read. */
    if (nsig < 1)
	sprintf(ts, "Record %s is unavailable\n", record);
    if (nsig < 1) {
#ifdef NOTICE
	Xv_notice notice = xv_create((Frame)frame, NOTICE,
				     XV_SHOW, TRUE,
#else
	(void)notice_prompt((Frame)frame, (Event *)NULL,
#endif
		      NOTICE_MESSAGE_STRINGS,
		      ts, 0,
		      NOTICE_BUTTON_YES, "Continue", 0);
#ifdef NOTICE
	xv_destroy_safe(notice);
#endif
	return (0);
    }

    /* If the record has a low sampling rate, use coarse time scale and grid
       mode. */
    if (freq <= 10.0) {
	tsa_index = coarse_tsa_index;
	grid_mode = coarse_grid_mode;
	mode_undo();
	set_modes();
    }
    else {
	tsa_index = fine_tsa_index;
	grid_mode = fine_grid_mode;
	mode_undo();
	set_modes();
    }

    /* Set signal name pointers.  Shorten the conventional "record x, signal n"
       to "signal n". */
    sprintf(ts, "record %s, ", record);
    tl = strlen(ts);
    for (i = 0; i < nsig; i++) {
	if (strncmp(df[i].desc, ts, tl) == 0)
	    signame[i] = df[i].desc + tl;
	else
	    signame[i] = df[i].desc;
	if (df[i].units == NULL || *df[i].units == '\0')
	    sigunits[i] = "mV";
	else
	    sigunits[i] = df[i].units;
	/* Replace any unspecified signal gains with the default gain from
	   wfdb.h;  the units of gain are ADC units per physical unit. */
	if (df[i].gain == 0) {
	    calibrated[i] = 0;
	    df[i].gain = WFDB_DEFGAIN;
	}
	else
	    calibrated[i] = 1;

    }	

    /* Set range for signal selection on analyze panel. */
    reset_maxsig();

    /* Initialize the signal list unless the new record name matches the
       old one. */
    if (rebuild_list) {
	for (i = 0; i < nsig; i++)
	    siglist[i] = i;
	siglistlen = nsig;
	reset_siglist();
    }

    /* Calculate the base levels (in display units) for each signal, and for
       annotation display. */
    set_baselines();
    vscale[0] = 0.;	/* force clear_cache() -- see calibrate() */
    calibrate();

    /* Rebuild the level window (see edit.c) */
    recreate_level_popup();
    return (1);
}

/* Set_baselines() determines the ordinates for the signal baselines and for
   annotation display.  Note that the signals are drawn centered about the
   calculated baselines (i.e., the baselines bear no fixed relationship to
   the sample values). */

void set_baselines()
{
    int i;

    if (sig_mode == 0)
	for (i = 0; i < nsig; i++)
	    base[i] = canvas_height*(2*i+1.)/(2.*nsig);
    else
	for (i = 0; i < siglistlen; i++)
	    base[i] = canvas_height*(2*i+1.)/(2.*siglistlen);
    abase = (nsig > 1) ? (base[i/2] + base[i/2-1])/2 : canvas_height*4/5;
}

/* Calibrate() sets scales for the display.  Ordinate (amplitude) scaling
   is determined for each signal from the gain recorded in the header file,
   and from the display scales in the calibration specification file, but
   calibrate() scales the abscissa (time) for all signals based on the
   sampling frequency for signal 0. */

void calibrate()
{
    int i;
    extern char *getenv();
    struct WFDB_calinfo ci;

    /* Vscale is a multiplicative scale factor that converts sample values to
       window ordinates.  Since window ordinates are inverted, vscale includes
       a factor of -1. */
    if (vscale[0] == 0.0) {
	clear_cache();

	/* If specified, read the calibration file to get standard scales. */
	if ((cfname == (char *)NULL) && (cfname = getenv("WFDBCAL")))
	    calopen(cfname);

	for (i = 0; i < nsig; i++) {
	    vscale[i] = - millivolts(1) / df[i].gain;
	    dc_coupled[i] = 0;
	    if (getcal(df[i].desc, df[i].units, &ci) == 0 && ci.scale != 0.0) {
		vscale[i] /= ci.scale;
		if (ci.caltype & 1) {
		    dc_coupled[i] = 1;
		    sigbase[i] = df[i].baseline;
		    if (blabel[i] = (char *)malloc(strlen(ci.units) +
						   strlen(df[i].desc) + 6))
			sprintf(blabel[i], "0 %s (%s)", ci.units, df[i].desc);
		}
	    }
	}
    }
    /* Tscale is a multiplicative scale factor that converts sample intervals
       to window abscissas. */
    if (freq == 0.0) freq = WFDB_DEFFREQ;
    nsamp = canvas_width_sec * freq;
    tscale = seconds(1) / freq;
}
