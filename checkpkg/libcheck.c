/* file: wfdbcheck.c	G. Moody       	7 September 2001

-------------------------------------------------------------------------------
wfdbcheck: test WFDB library
Copyright (C) 2001 George B. Moody

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
#ifdef __STDC__
# include <stdlib.h>
#else
extern void exit();
# ifndef NOMALLOC_H
# include <malloc.h>
# else
extern char *malloc();
# endif
#endif
#ifndef BSD
# include <string.h>
#else
# include <strings.h>
#endif

#include <wfdb/wfdb.h>

char *pname, *prog_name();
int n, nsig, i, framelen, errors = 0, stat, vflag = 1;
char headerversion[40];
char *libversion;
char *p, *defpath, *dbpath, *localrec, *remrec;
WFDB_Anninfo aiarray[2];
WFDB_Siginfo *si;
WFDB_Sample *vector;

main(argc, argv)
int argc;
char *argv[];
{
  void help();

  if (argc > 1) {
    if (strcmp(argv[1], "-v") == 0) vflag = 1;
    else { help(); exit(1); }
  }

  if ((p = wfdberror()) == NULL) {
    fprintf(stderr, "Error: wfdberror() did not return a version string\n");
    p = "unknown version of the WFDB library";
    errors++;
  }
  libversion = calloc(sizeof(char), strlen(p)+1);
  strcpy(libversion, p);

  /* Print library version number and date. */
  fprintf(stderr, "Checking %s\n", libversion = wfdberror());

  /* Check that the installed <wfdb.h> matches the library. */
  sprintf(headerversion, "WFDB library version %d.%d.%d",
	  WFDB_MAJOR, WFDB_MINOR, WFDB_RELEASE);
  if (strncmp(libversion, headerversion, strlen(headerversion))) {
    fprintf(stderr, "Error: Library version does not match <wfdb.h>\n"
                    "       (<wfdb.h> is from %s)\n", headerversion);
    errors++;
  }
  else if (vflag)
    fprintf(stderr, "[OK]:  Library version matches <wfdb.h>\n");

  /* If in verbose mode, print library defaults. */
  if (vflag) {
    static int format[WFDB_NFMTS] = WFDB_FMT_LIST;

    fprintf(stderr, "[OK]:  WFDB %s NETFILES\n", WFDB_NETFILES ? "supports" :
	    "does not support");
    fprintf(stderr, "[OK]:  WFDB_MAXANN = %d\n", WFDB_MAXANN);
    fprintf(stderr, "[OK]:  WFDB_MAXSIG = %d\n", WFDB_MAXSIG);
    fprintf(stderr, "[OK]:  WFDB_MAXSPF = %d\n", WFDB_MAXSPF);
    fprintf(stderr, "[OK]:  WFDB_MAXRNL = %d\n", WFDB_MAXRNL);
    fprintf(stderr, "[OK]:  WFDB_MAXUSL = %d\n", WFDB_MAXUSL);
    fprintf(stderr, "[OK]:  WFDB_MAXDSL = %d\n", WFDB_MAXDSL);
    fprintf(stderr, "[OK]:  Signal formats = {");
    for (i = 0; i < WFDB_NFMTS; i++)
      fprintf(stderr, "%d%s", format[i], (i < WFDB_NFMTS-1) ? ", " : "}\n");
    fprintf(stderr, "[OK]:  WFDB_DEFFREQ = %g\n",  WFDB_DEFFREQ);
    fprintf(stderr, "[OK]:  WFDB_DEFGAIN = %g\n",  WFDB_DEFGAIN);
    fprintf(stderr, "[OK]:  WFDB_DEFRES = %d\n",  WFDB_DEFRES);
  }

  /* Test the WFDB library functions. */

  localrec = "100s";
  remrec = "mitdb/100";
  aiarray[0].name = "atr"; aiarray[0].stat = WFDB_READ;
  aiarray[1].name = "chk"; aiarray[1].stat = WFDB_WRITE;

  /* *** getwfdb *** */
  if ((p = getwfdb()) == NULL) {
    fprintf(stderr, "Error: No default WFDB path defined\n");
    defpath = "";
    errors++;
  }
  else {
    defpath = calloc(sizeof(char), strlen(p)+1);
    strcpy(defpath, p);

    if (vflag)
      fprintf(stderr, "[OK]:  Default WFDB path = %s\n", defpath);
  }

  /* *** setwfdb *** */
  dbpath = calloc(sizeof(char), strlen(defpath)+10);
  sprintf(dbpath, "../data %s\n", defpath);
  setwfdb(dbpath);
  if ((p = getwfdb()) == NULL || strcmp(p, dbpath)) {
    fprintf(stderr, "Error: Could not set WFDB path\n");
    errors++;
  }
  else if (vflag)
    fprintf(stderr, "[OK]:  WFDB path modified successfully\n");

  /* Test I/O using the local record first. */
  check(localrec);

  /* Test I/O again using the remote record. */
  if (WFDB_NETFILES) {
    if (vflag)
      fprintf(stderr, "[OK]:  Repeating tests using NETFILES");
    if (strstr(defpath, "http://") == NULL) {
      fprintf(stderr,
       "\nWarning: default WFDB path does not include an http:// component\n");
      setwfdb(". http://www.physionet.org/physiobank/database");
      fprintf(stderr, "WFDB path has been set to: %s\n", getwfdb());
    }
    else if (vflag) {
      fprintf(stderr, " (reverting to default WFDB path)\n");
      setwfdb(defpath);
    }
    check(remrec);
  }

  /* If there were any errors detected by the WFDB library but not by this
     program, this test will pick them up. */
  if (errors == 0) {
    if ((p = wfdberror()) == NULL) {
      fprintf(stderr, "Error: wfdberror() did not return a version string\n");
      p = "unknown version of the WFDB library";
      errors++;
    }
    if (strcmp(libversion, p)) {
      fprintf(stderr, "Error: WFDB library error(s) not detected by %s\n",
	      pname);
      fprintf(stderr, " Last library error was: '%s'\n", p);
      errors++;
    }
    else if (vflag)
      fprintf(stderr, "[OK]:  no WFDB library errors\n");
  }

  /* Summarize the results and exit. */
  if (errors)
    fprintf(stderr, "%d error%s: test failed\n", errors, errors > 1 ? "s" :"");
  else if (vflag)
    fprintf(stderr, "no errors: test succeeded\n");
  exit(errors);
}

int check(char *record)
{
  WFDB_Frequency f;
  WFDB_Time t, tt;

  /* *** annopen *** */
  stat = annopen(record, aiarray, 1);
  if (stat) {
    fprintf(stderr,
	  "Error: annopen of 1 file returned %d (should have been 0)\n", stat);
    errors++;
  }
  else if (vflag)
    fprintf(stderr, "[OK]:  annopen of 1 file succeeded\n");

  wfdbquit();

  stat = annopen(record, aiarray, 2);
  if (stat) {
    fprintf(stderr,
	 "Error: annopen of 2 files returned %d (should have been 0)\n", stat);
    errors++;
  }
  else if (vflag)
    fprintf(stderr, "[OK]:  annopen of 2 files succeeded\n");

  /* *** isigopen *** */
  /* Get the number of signals without opening any signal files. */
  n = isigopen(record, NULL, 0);
  if (n != 2) {
    fprintf(stderr,
	    "Error: isigopen(%s, NULL, 0) returned %d (should have been 2)\n",
	    record, n);
    errors++;
  }
  else if (vflag)
    fprintf(stderr, "[OK]:  isigopen(%s, NULL, 0) succeeded\n");
	
  /* Allocate WFDB_Siginfo structures before calling isigopen again. */
  si = calloc(n, sizeof(WFDB_Siginfo));
  nsig = isigopen(record, si, n);
  /* Note that nsig equals n only if all signals were readable. */

  /* Get the number of samples per frame. */
  for (i = framelen = 0; i < nsig; i++)
    framelen += si[i].spf;
  /* Allocate WFDB_Samples before calling getframe. */
  vector = calloc(framelen, sizeof(WFDB_Sample));

  for (t = 0L; t < 5L; t++) {
    if ((n = getframe(vector)) != nsig) {
      fprintf(stderr, "Error: getframe returned %d (should have been %d)\n",
	      n, nsig);
      errors++;
      break;
    }
    else if (vflag) {
      fprintf(stderr, "[OK]:  (at %s) getframe returned {", mstimstr(t));
      for (i = 0; i < framelen; i++)
	fprintf(stderr, "%5d%s", vector[i], i < framelen-1 ? ", " : "}\n");
    }
  }

  /* *** sampfreq *** */
  f = sampfreq(NULL);
  if (f != 360.0) {
    fprintf(stderr, "Error: sampfreq returned %g (should have been 360)\n", f);
    errors++;
  }
  else if (vflag)
    fprintf(stderr, "[OK]:  sampfreq returned %g\n", f);

  /* *** strtim *** */
  t = strtim("0:20");
  if (t != (WFDB_Time)(20.0 * f)) {
    fprintf(stderr, "Error: strtim returned %ld (should have been %ld)\n", t,
	    (WFDB_Time)(20.0 * f));
    errors++;
  }
  else if (vflag)
    fprintf(stderr, "[OK]:  strtim returned %ld\n", t);
  
  /* *** isigsettime *** */
  stat = isigsettime(t);
  if (stat) {
    fprintf(stderr, "Error: isigsettime returned %d (should have been 0)\n",
	    stat);
    errors++;
  }
  else if (vflag)
    fprintf(stderr, "[OK]:  isigsettime skipping forward to %s\n",
	    timstr(t));

  /* *** getvec *** */
  for (tt = t; t < tt+5L; t++) {
    if ((n = getvec(vector)) != nsig) {
      fprintf(stderr, "Error: getvec returned %d (should have been %d)\n",
	      n, nsig);
      errors++;
      break;
    }
    else if (vflag) {
      fprintf(stderr, "[OK]:  (at %s) getvec returned {", mstimstr(t));
      for (i = 0; i < nsig; i++)
	fprintf(stderr, "%5d%s", vector[i], i < nsig-1 ? ", " : "} [");
      for (i = 0; i < nsig; i++)
	fprintf(stderr, "%5.3lf%s", aduphys(i, vector[i]), i < nsig-1?", ":"]\n");
    }
  }

  /* *** isigsettime *** */
  t = tt-1; /* try a backward skip, to one sample before the previous set */
  stat = isigsettime(t);
  if (stat) {
    fprintf(stderr, "Error: isigsettime returned %d (should have been 0)\n",
	    stat);
    errors++;
  }
  else if (vflag)
    fprintf(stderr, "[OK]:  isigsettime skipping backward to %s\n",
	    mstimstr(t));

  /* *** getframe, again *** */
  for ( ; t < tt+5L; t++) {
    if ((n = getframe(vector)) != nsig) {
      fprintf(stderr, "Error: getframe returned %d (should have been %d)\n",
	      n, nsig);
      errors++;
      break;
    }
    else if (vflag) {
      fprintf(stderr, "[OK]:  (at %s) getframe returned {", mstimstr(t));
      for (i = 0; i < framelen; i++)
	fprintf(stderr, "%5d%s", vector[i], i < framelen-1 ? ", " : "}\n");
    }
  }

#if 0
extern FINT osigopen(char *record, WFDB_Siginfo *siarray, unsigned int nsig);
extern FINT osigfopen(WFDB_Siginfo *siarray, unsigned int nsig);
extern FINT wfdbinit(char *record, WFDB_Anninfo *aiarray, unsigned int nann,
                  WFDB_Siginfo *siarray, unsigned int nsig);
extern FINT getspf(void);
extern FVOID setgvmode(int mode);
extern FINT putvec(WFDB_Sample *vector);
extern FINT getann(WFDB_Annotator a, WFDB_Annotation *annot);
extern FINT ungetann(WFDB_Annotator a, WFDB_Annotation *annot);
extern FINT putann(WFDB_Annotator a, WFDB_Annotation *annot);
extern FINT isgsettime(WFDB_Group g, WFDB_Time t);
extern FINT iannsettime(WFDB_Time t);
extern FSTRING ecgstr(int annotation_code);
extern FINT strecg(char *annotation_mnemonic_string);
extern FINT setecgstr(int annotation_code, char *annotation_mnemonic_string);
extern FSTRING annstr(int annotation_code);
extern FINT strann(char *annotation_mnemonic_string);
extern FINT setannstr(int annotation_code, char *annotation_mnemonic_string);
extern FSTRING anndesc(int annotation_code);
extern FINT setanndesc(int annotation_code, char *annotation_description);
extern FVOID iannclose(WFDB_Annotator a);
extern FVOID oannclose(WFDB_Annotator a);
extern FSTRING timstr(WFDB_Time t);
extern FSTRING datstr(WFDB_Date d);
extern FDATE strdat(char *date_string);
extern FINT adumuv(WFDB_Signal s, WFDB_Sample a);
extern FSAMPLE muvadu(WFDB_Signal s, int microvolts);
extern FSAMPLE physadu(WFDB_Signal s, double v);
extern FINT calopen(char *calibration_filename);
extern FINT getcal(char *description, char *units, WFDB_Calinfo *cal);
extern FINT putcal(WFDB_Calinfo *cal);
extern FINT newcal(char *calibration_filename);
extern FVOID flushcal(void);
extern FSTRING getinfo(char *record);
extern FINT putinfo(char *info);
extern FINT newheader(char *record);
extern FINT setheader(char *record, WFDB_Siginfo *siarray, unsigned int nsig);
extern FINT setmsheader(char *record, char **segnames, unsigned int nsegments);
extern FINT wfdbgetskew(WFDB_Signal s);
extern FVOID wfdbsetskew(WFDB_Signal s, int skew);
extern FLONGINT wfdbgetstart(WFDB_Signal s);
extern FVOID wfdbsetstart(WFDB_Signal s, long bytes);
extern FVOID wfdbquit(void);
extern FFREQUENCY sampfreq(char *record);
extern FINT setsampfreq(WFDB_Frequency sampling_frequency);
extern FFREQUENCY getcfreq(void);
extern FVOID setcfreq(WFDB_Frequency counter_frequency);
extern FDOUBLE getbasecount(void);
extern FVOID setbasecount(double count);
extern FINT setbasetime(char *time_string);
extern FVOID wfdbquiet(void);
extern FVOID wfdbverbose(void);
extern FINT setibsize(int input_buffer_size);
extern FINT setobsize(int output_buffer_size);
extern FSTRING wfdbfile(char *file_type, char *record);
extern FVOID wfdbflush(void);
#endif
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
 "usage: %s [OPTIONS ...]\n",
 " -v          verbose mode",
 NULL
};

void help()
{
    int i;

    (void)fprintf(stderr, help_strings[0], pname);
    for (i = 1; help_strings[i] != NULL; i++)
	(void)fprintf(stderr, "%s\n", help_strings[i]);
}
