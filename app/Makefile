# file: Makefile	G. Moody	 9 June 1983
#			Last revised:	25 August 1999
# UNIX 'make' description file for WFDB applications
#
# -----------------------------------------------------------------------------
# WFDB applications: programs for working with annotated signals
# Copyright (C) 1999 George B. Moody
#
# These programs are free software; you can redistribute them and/or modify
# them under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# These programs are distributed in the hope that they will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# these programs; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# You may contact the author by e-mail (george@mit.edu) or postal mail
# (MIT Room E25-505A, Cambridge, MA 02139 USA).  For updates to this software,
# please visit the author's web site (http://ecg.mit.edu/).
# _____________________________________________________________________________
#
# This file is used with the UNIX `make' command to install the standard
# applications that come with the WFDB software package.  Before using it
# for the first time, check that the site-specific variables below are
# appropriate for your system.  To build and install the applications, just
# type `make' (from within this directory).  To print a set of source listings,
# type `make listing'.

# Site-specific variables
# -----------------------

# CC is the name of your C compiler.  These applications can be compiled with
# K&R C compilers (`cc' on most if not all UNIX systems) as well as with ANSI
# C compilers such as GNU C (`gcc').  To use the K&R C compiler available on
# most UNIX systems, uncomment the next line.
CC = cc
# For `gcc', uncomment the next line (and see CFLAGS below).
# CC = gcc

# INCDIR is the parent of the directory that contains the WFDB include (*.h)
# files.  (These files should have been installed in $(INCDIR)/wfdb;  see
# ../lib/Makefile.)
INCDIR = /usr/include

# If you trust your C compiler's optimizer, you may wish to add -O to CFLAGS;
# for debugging, add -g to CFLAGS.  (If you use `gcc', you may use both;  most
# other C compilers do not permit you to do so.)  Add -DNOMALLOC_H if you have
# neither `stdlib.h' nor `malloc.h' (see ../lib/wfdblib.h).  Add -DNOSTRSTR if
# you don't have the ANSI C `strstr' function (check your system's `string.h'
# or `strings.h' file, normally in /usr/include; see `pschart.c' and `psfd.c').
CFLAGS = -O -I$(INCDIR)

# LIBDIR is the directory that contains the WFDB library (see ../lib/Makefile).
LIBDIR = /usr/lib

# LDFLAGS is appended to the C compiler command line to specify loading the
# WFDB library.  Unless you have changed the value of WFDBLIB in the `Makefile'
# for the WFDB library, `-lwfdb' should be correct.
LDFLAGS = -L$(LIBDIR) -lwfdb

# BINDIR specifies the directory in which the applications will be installed;
# it should be a directory in the PATH of those who will use the applications.
# You will need to have write permission in BINDIR.  Users of this software
# will need to have search (execute) permission in BINDIR.
BINDIR = /usr/local/bin

# DBDIR specifies the name of a directory in which to install the contents
# of the `data' directory.
DBDIR = $(INSTALL)/database

# PSPDIR specifies the directory in which PostScript prolog files are kept.
# (You may use any directory for this purpose;  if you have other PostScript
# applications, you may wish to keep all of your prolog files in the same
# directory, often /usr/local/lib/ps).  You will need to have write permission
# in PSPDIR.  Users of this software will need to have search (execute)
# permission in PSPDIR.
PSPDIR = /usr/local/lib/ps

# PRINT is the name of the program used to produce listings (including any
# options for the desired formatting).
PRINT = lpr

# SETXPERMISSIONS is the command needed to make the installed files accessible
# to those who will use them.  The value given below makes them readable and
# executable by everyone, and writeable by the owner only.  (If you perform the
# installation as `root', `root' is the owner of the installed files.)
SETXPERMISSIONS = chmod 755

# SETPERMISSIONS is similarly used to make installed data files accessible.
# These files should not be marked as executable.
SETPERMISSIONS = chmod 644

# STRIP is the command used to compact the compiled binaries by removing their
# symbol tables.
STRIP = strip
# To retain the symbol tables for debugging, comment out the previous line, and
# uncomment the next line.
# STRIP = :

# It should not be necessary to modify anything below this line.
# -----------------------------------------------------------------------------

CFILES = bxb.c calibrate.c ecgeval.c epic.c fir.c ihr.c mfilt.c mrgann.c \
 mxm.c nst.c plotstm.c pschart.c psfd.c rdann.c rdsamp.c rxr.c sampfreq.c \
 sample.c sigamp.c skewedit.c snip.c sortann.c sqrs.c sqrs125.c sumann.c \
 sumstats.c tach.c view.c vsetup.c wfdbcollate.c wfdbdesc.c wfdbwhich.c \
 wrann.c wrsamp.c wvscript.c xform.c
XFILES = bxb calibrate ecgeval epic fir ihr mfilt mrgann \
 mxm nst plotstm pschart psfd rdann rdsamp rxr sampfreq \
 sigamp skewedit snip sortann sqrs sqrs125 sumann \
 sumstats tach wfdbcollate wfdbdesc wfdbwhich \
 wrann wrsamp xform
SCRIPTS = cshsetwfdb setwfdb
PSFILES = pschart.pro psfd.pro
OTHERFILES = cshsetwfdb setwfdb setwfdb.bat sample8.hea
MFILES = Makefile Makefile-dos-gcc makefile.dos

# General rule for compiling C sources into executable files.  This is
# redundant for most versions of `make', but at least one System V version
# needs it.
.c:
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# `make' or `make install':  build and install applications, clean up
install:	all $(BINDIR) $(PSPDIR) scripts
	cp $(XFILES) $(BINDIR)
	cd $(BINDIR); $(SETXPERMISSIONS) $(XFILES)
	cp $(PSFILES) $(PSPDIR)
	cd $(PSPDIR); $(SETPERMISSIONS) $(PSFILES)
	$(MAKE) clean

# `make all': build applications
all:	$(XFILES)
	$(STRIP) $(XFILES)

# `make scripts': install customized scripts for setting WFDB path
scripts:
	sed s+/usr/local/database+$(DBDIR)+g <setwfdb >$(BINDIR)/setwfdb
	sed s+/usr/local/database+$(DBDIR)+g <cshsetwfdb >$(BINDIR)/cshsetwfdb
	$(SETPERMISSIONS) $(SCRIPTS)

$(BINDIR):
	mkdir -p $(BINDIR)

$(PSPDIR):
	mkdir -p $(PSPDIR)

# `make clean':  remove intermediate and backup files
clean:
	rm -f $(XFILES) *.o *~

# `make listing':  print a listing of WFDB applications sources
listing:
	$(PRINT) README $(MFILES) $(CFILES) $(PSFILES) $(OTHERFILES)

# Rules for compiling applications that require non-standard options

bxb:		bxb.c
	$(CC) $(CFLAGS) bxb.c -o $@ $(LDFLAGS) -lm
mxm:		mxm.c
	$(CC) $(CFLAGS) mxm.c -o $@ $(LDFLAGS) -lm
nst:		nst.c
	$(CC) $(CFLAGS) nst.c -o $@ $(LDFLAGS) -lm
plotstm:	plotstm.c
	$(CC) $(CFLAGS) plotstm.c -o $@
pschart:	pschart.c
	$(CC) $(CFLAGS) -DPROLOG=\"$(PSPDIR)/pschart.pro\" pschart.c -o $@ \
          $(LDFLAGS)
psfd:		psfd.c
	$(CC) $(CFLAGS) -DPROLOG=\"$(PSPDIR)/psfd.pro\" psfd.c -o $@ $(LDFLAGS)
sigamp:		sigamp.c
	$(CC) $(CFLAGS) sigamp.c -o $@ $(LDFLAGS) -lm
