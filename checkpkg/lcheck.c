/* file: wfdbcheck.c	G. Moody       	7 September 2001
			Last revised:	9 September 2001
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

#if 0
/* This program does not (yet) test the following WFDB library functions. */
extern FINT osigopen(char *record, WFDB_Siginfo *siarray, unsigned int nsig);
extern FINT wfdbinit(char *record, WFDB_Anninfo *aiarray, unsigned int nann,
                  WFDB_Siginfo *siarray, unsigned int nsig);
extern FINT getspf(void);
extern FVOID setgvmode(int mode);
extern FINT ungetann(WFDB_Annotator a, WFDB_Annotation *annot);
extern FINT isgsettime(WFDB_Group g, WFDB_Time t);
extern FSTRING ecgstr(int annotation_code);
extern FINT strecg(char *annotation_mnemonic_string);
extern FINT setecgstr(int annotation_code, char *annotation_mnemonic_string);
extern FINT strann(char *annotation_mnemonic_string);
extern FINT setannstr(int annotation_code, char *annotation_mnemonic_string);
extern FSTRING anndesc(int annotation_code);
extern FINT setanndesc(int annotation_code, char *annotation_description);
extern FVOID iannclose(WFDB_Annotator a);
extern FVOID oannclose(WFDB_Annotator a);
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
extern FINT setheader(char *record, WFDB_Siginfo *siarray, unsigned int nsig);
extern FINT setmsheader(char *record, char **segnames, unsigned int nsegments);
extern FINT wfdbgetskew(WFDB_Signal s);
extern FVOID wfdbsetskew(WFDB_Signal s, int skew);
extern FLONGINT wfdbgetstart(WFDB_Signal s);
extern FVOID wfdbsetstart(WFDB_Signal s, long bytes);
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

char *info, *pname, *prog_name();
int n, nsig, i, j, framelen, errors = 0, stat, vflag = 0;
char headerversion[40];
char *libversion;
char *p, *defpath, *dbpath;
WFDB_Anninfo aiarray[2];
WFDB_Annotation annot;
WFDB_Siginfo *si;
WFDB_Sample *vector;
void help();

main(argc, argv)
int argc;
char *argv[];
{

  pname = prog_name();
  if (argc > 1) {
    if (strcmp(argv[1], "-v") == 0) vflag = 1;
    else { help(); exit(1); }
  }

  if ((p = wfdberror()) == NULL) {
    printf("Error: wfdberror() did not return a version string\n");
    p = "unknown version of the WFDB library";
    errors++;
  }
  libversion = calloc(sizeof(char), strlen(p)+1);
  strcpy(libversion, p);

  /* Print the library version number and date. */
  fprintf(stderr, "Testing %s", libversion = wfdberror());

  /* Check that the installed <wfdb.h> matches the library. */
  sprintf(headerversion, "WFDB library version %d.%d.%d",
	  WFDB_MAJOR, WFDB_MINOR, WFDB_RELEASE);
  if (strncmp(libversion, headerversion, strlen(headerversion))) {
    printf("Error: Library version does not match <wfdb.h>\n"
                    "       (<wfdb.h> is from %s)\n", headerversion);
    errors++;
  }
  else if (vflag)
    printf("[OK]:  Library version matches <wfdb.h>\n");

  /* If in verbose mode, print library defaults. */
  if (vflag) {
    static int format[WFDB_NFMTS] = WFDB_FMT_LIST;

    printf("[OK]:  WFDB %s NETFILES\n", WFDB_NETFILES ? "supports" :
	    "does not support");
    printf("[OK]:  WFDB_MAXANN = %d\n", WFDB_MAXANN);
    printf("[OK]:  WFDB_MAXSIG = %d\n", WFDB_MAXSIG);
    printf("[OK]:  WFDB_MAXSPF = %d\n", WFDB_MAXSPF);
    printf("[OK]:  WFDB_MAXRNL = %d\n", WFDB_MAXRNL);
    printf("[OK]:  WFDB_MAXUSL = %d\n", WFDB_MAXUSL);
    printf("[OK]:  WFDB_MAXDSL = %d\n", WFDB_MAXDSL);
    printf("[OK]:  Signal formats = {");
    for (i = 0; i < WFDB_NFMTS; i++)
      printf("%d%s", format[i], (i < WFDB_NFMTS-1) ? ", " : "}\n");
    printf("[OK]:  WFDB_DEFFREQ = %g\n",  WFDB_DEFFREQ);
    printf("[OK]:  WFDB_DEFGAIN = %g\n",  WFDB_DEFGAIN);
    printf("[OK]:  WFDB_DEFRES = %d\n",  WFDB_DEFRES);
  }

  /* Test the WFDB library functions. */
  aiarray[0].name = "atr"; aiarray[0].stat = WFDB_READ;
  aiarray[1].name = "chk"; aiarray[1].stat = WFDB_WRITE;

  /* *** getwfdb *** */
  if ((p = getwfdb()) == NULL) {
    printf("Error: No default WFDB path defined\n");
    defpath = "";
    errors++;
  }
  else {
    defpath = calloc(sizeof(char), strlen(p)+1);
    strcpy(defpath, p);

    if (vflag)
      printf("[OK]:  Default WFDB path = %s\n", defpath);
  }

  /* *** setwfdb *** */
  dbpath = calloc(sizeof(char), strlen(defpath)+10);
  sprintf(dbpath, "../data %s\n", defpath);
  setwfdb(dbpath);
  if ((p = getwfdb()) == NULL || strcmp(p, dbpath)) {
    printf("Error: Could not set WFDB path\n");
    errors++;
  }
  else if (vflag)
    printf("[OK]:  WFDB path modified successfully\n");

  /* Test I/O using the local record first. */
  check("100s", "100x");

  /* Test I/O again using the remote record. */
  if (WFDB_NETFILES) {
    if (vflag)
      printf("[OK]:  Repeating tests using NETFILES");
    if (strstr(defpath, "http://") == NULL) {
      fprintf(stderr,
       "\nWarning: default WFDB path does not include an http:// component\n");
      setwfdb(". http://www.physionet.org/physiobank/database");
      printf("WFDB path has been set to: %s\n", getwfdb());
    }
    else if (vflag) {
      printf(" (reverting to default WFDB path)\n");
      setwfdb(defpath);
    }
    check("udb/100s", "udb/100x");
  }

  /* If there were any errors detected by the WFDB library but not by this
     program, this test will pick them up. */
  if (errors == 0) {
    if ((p = wfdberror()) == NULL) {
      printf("Error: wfdberror() did not return a version string\n");
      p = "unknown version of the WFDB library";
      errors++;
    }
    if (strcmp(libversion, p)) {
      printf("Error: WFDB library error(s) not detected by %s\n",
	      pname);
      printf(" Last library error was: '%s'\n", p);
      errors++;
    }
    else if (vflag)
      printf("[OK]:  no WFDB library errors\n");
  }

  /* Summarize the results and exit. */
  if (errors)
    printf("%d error%s: test failed\n", errors, errors > 1 ? "s" :"");
  else if (vflag)
    printf("no errors: test succeeded\n");
  exit(errors);
}

int check(char *record, char *orec)
{
  WFDB_Frequency f;
  WFDB_Time t, tt;

  /* *** sampfreq *** */
  if ((f = sampfreq(NULL)) != 0.0) {
    printf("Error: sampfreq(NULL) returned %g (should have been 0)\n", f);
    errors++;
  }
  else if (vflag)
    printf("[OK]:  sampfreq(NULL) returned %g\n", f);

  /* *** setsampfreq *** */
  if (stat = setsampfreq(100.0)) {
    printf("Error: setsampfreq returned %d (should have been 0)\n", stat);
    errors++;
  }
  if (sampfreq(NULL) != 100.0) {
    printf("Error: failed to set sampling frequency using setsampfreq\n");
    errors++;
  }
  else if (vflag)
    printf("[OK]:  setsampfreq changed sampling frequency successfully\n");

  /* *** sampfreq, again *** */
  if ((f = sampfreq(record)) != 360.0) {
    printf("Error: sampfreq(%s) returned %g (should have been 360)\n",
	   record, f);
    errors++;
  }
  else if (vflag)
    printf("[OK]:  sampfreq(%s) returned %g\n", record, f);

  /* *** annopen *** */
  stat = annopen(record, aiarray, 1);
  if (stat) {
    fprintf(stderr,
	  "Error: annopen of 1 file returned %d (should have been 0)\n", stat);
    errors++;
  }
  else if (vflag)
    printf("[OK]:  annopen of 1 file succeeded\n");

  stat = annopen(record, aiarray, 2);
  if (stat) {
    fprintf(stderr,
	 "Error: annopen of 2 files returned %d (should have been 0)\n", stat);
    errors++;
  }
  else if (vflag)
    printf("[OK]:  annopen of 2 files succeeded\n");

  /* *** strtim *** */
  t = strtim("0:5");
  if (t != (WFDB_Time)(5.0 * f)) {
    printf("Error: strtim returned %ld (should have been %ld)\n", t,
	    (WFDB_Time)(5.0 * f));
    errors++;
  }
  else if (vflag)
    printf("[OK]:  strtim returned %ld\n", t);
  
  /* *** iannsettime *** */
  stat = iannsettime(t);
  if (stat) {
    printf("Error: iannsettime returned %d (should have been 0)\n",
	    stat);
    errors++;
  }
  else if (vflag)
    printf("[OK]:  iannsettime skipping forward to %s\n",
	    timstr(t));

  /* *** getann, annstr, mstimstr *** */
  for (i = 0; i < 5; i++) {
    stat = getann(0, &annot);
    if (stat != 0 && stat != -1) {
      printf("Error: getann returned %d (should have been 0 or -1)\n",
	     stat);
      errors++;
    }
    else if (vflag)
      printf("[OK]:  getann read: {%s %d %d %d} at %s (%ld)\n",
	     annstr(annot.anntyp), annot.subtyp, annot.chan, annot.num,
	     mstimstr(annot.time), annot.time);
  }

  /* *** iannsettime, again *** */
  stat = iannsettime(0L);
  if (stat) {
    printf("Error: iannsettime returned %d (should have been 0)\n",
	    stat);
    errors++;
  }
  else if (vflag)
    printf("[OK]:  iannsettime skipping backward to %s\n",
	    timstr(0L));

  /* *** getann, putann *** */
  i = j = stat = 0;
  while (stat == 0) {
    stat = getann(0, &annot);
    if (stat != 0 && stat != -1) {
      printf("Error: getann returned %d (should have been 0 or -1)\n",
	     stat);
      errors++;
    }
    else if (stat == 0) {
      i++;
      stat = putann(0, &annot);
      if (stat != 0) {
	printf("Error: putann returned %d (should have been 0)\n",
	       stat);
	errors++;
      }
      else j++;
    }
  }
  if (vflag)
    printf("[OK]:  %d annotations read, %d written\n", i, j);

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
    printf("[OK]:  isigopen(%s, NULL, 0) succeeded\n");
	
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
      printf("Error: getframe returned %d (should have been %d)\n",
	      n, nsig);
      errors++;
      break;
    }
    else if (vflag) {
      printf("[OK]:  (at %s) getframe returned {", mstimstr(t));
      for (i = 0; i < framelen; i++)
	printf("%5d%s", vector[i], i < framelen-1 ? ", " : "}\n");
    }
  }

  /* *** sampfreq *** */
  f = sampfreq(NULL);
  if (f != 360.0) {
    printf("Error: sampfreq returned %g (should have been 360)\n", f);
    errors++;
  }
  else if (vflag)
    printf("[OK]:  sampfreq returned %g\n", f);

  /* *** strtim *** */
  t = strtim("0:20");
  if (t != (WFDB_Time)(20.0 * f)) {
    printf("Error: strtim returned %ld (should have been %ld)\n", t,
	    (WFDB_Time)(20.0 * f));
    errors++;
  }
  else if (vflag)
    printf("[OK]:  strtim returned %ld\n", t);
  
  /* *** isigsettime *** */
  stat = isigsettime(t);
  if (stat) {
    printf("Error: isigsettime returned %d (should have been 0)\n",
	    stat);
    errors++;
  }
  else if (vflag)
    printf("[OK]:  isigsettime skipping forward to %s\n",
	    timstr(t));

  /* *** getvec *** */
  for (tt = t; t < tt+5L; t++) {
    if ((n = getvec(vector)) != nsig) {
      printf("Error: getvec returned %d (should have been %d)\n",
	      n, nsig);
      errors++;
      break;
    }
    else if (vflag) {
      printf("[OK]:  (at %s) getvec returned {", mstimstr(t));
      for (i = 0; i < nsig; i++)
	printf("%5d%s", vector[i], i < nsig-1 ? ", " : "} [");
      for (i = 0; i < nsig; i++)
	printf("%5.3lf%s", aduphys(i, vector[i]), i < nsig-1?", ":"]\n");
    }
  }

  /* *** isigsettime *** */
  t = tt-1; /* try a backward skip, to one sample before the previous set */
  stat = isigsettime(t);
  if (stat) {
    printf("Error: isigsettime returned %d (should have been 0)\n",
	    stat);
    errors++;
  }
  else if (vflag)
    printf("[OK]:  isigsettime skipping backward to %s\n",
	    mstimstr(t));

  /* *** getframe, again *** */
  for ( ; t < tt+5L; t++) {
    if ((n = getframe(vector)) != nsig) {
      printf("Error: getframe returned %d (should have been %d)\n",
	      n, nsig);
      errors++;
      break;
    }
    else if (vflag) {
      printf("[OK]:  (at %s) getframe returned {", mstimstr(t));
      for (i = 0; i < framelen; i++)
	printf("%5d%s", vector[i], i < framelen-1 ? ", " : "}\n");
    }
  }

  /* Now return to the beginning of the record and copy it. */
  stat = isigsettime(t = 0L);
  if (stat) {
    printf("Error: isigsettime returned %d (should have been 0)\n",
	    stat);
    errors++;
  }
  else if (vflag)
    printf("[OK]:  isigsettime skipping backward to %s\n",
	    mstimstr(t));

  /* *** osigfopen *** */
  for (i = 0; i < nsig; i++) {
    si[i].fname = realloc(si[i].fname, strlen(orec) + 5);
    sprintf(si[i].fname, "%s.dat", orec);
  }
  stat = osigfopen(si, nsig);
  if (stat != nsig) {
      printf("Error: osigfopen returned %d (should have been %d)\n",
	     stat, nsig);
      errors++;
  }
  else if (vflag)
      printf("[OK]:  osigfopen returned %d\n", stat);

  /* *** getframe (again), putvec *** */
  while ((n = getframe(vector)) == nsig) {
    t++;
    if ((stat = putvec(vector)) != nsig) {
      printf("Error: putvec returned %d (should have been %d)\n",
	     stat, nsig);
      errors++;
      break;
    }
  }
  if (n != -1) {	/* some error occurred while reading samples */
    printf("Error: getframe returned %d (should have been %d) at %s\n",
	   n, nsig, mstimstr(t));
    errors++;
  }
  else if (vflag)	/* getframe reached EOF, checksums OK */
    printf("[OK]:  getframe read %ld samples\n", t);
  if (stat != nsig) {	/* some error occurred while writing samples */
    printf("Error: putvec returned %d (should have been %d) at %s\n",
	   stat, nsig, mstimstr(t));
    errors++;
  }
  else if (vflag)	/* putvec wrote all samples without apparent error */
    printf("[OK]:  putvec wrote %ld samples\n", t);

  /* *** newheader *** */
  stat = newheader(orec);
  if (stat) {	/* some error occurred while writing the header */
    printf("Error: newheader returned %d (should have been 0)\n", stat);
    errors++;
  }
  else if (vflag)	/* putvec wrote all samples without apparent error */
    printf("[OK]:  newheader created header for output record %s\n", orec);

  /* *** getinfo, putinfo *** */
  n = 0;
  if (info = getinfo(record)) {
    do {
      stat = putinfo(info);
      if (stat) {
	printf("Error: putinfo returned %d (should have been 0)\n", stat);
	errors++;
      }
      else
	  n++;
    } while (info = getinfo(NULL));
    if (vflag)
	printf("[OK]:  %d info strings copied to record %s header\n", n, orec);
  }

  wfdbquit();
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
