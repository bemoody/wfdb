# file: Makefile	G. Moody	2 November 1989
#			Last revised:	 18 July 1999
# UNIX 'make' description file for WFDB format-conversion applications
#
# -----------------------------------------------------------------------------
# WFDB format-conversion applications
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
# please visit PhysioNet (http://www.physionet.org/).
# _____________________________________________________________________________
#
# This file is used with the UNIX `make' command to install the format-
# conversion applications that come with the WFDB software package.  Before
# using it for the first time, check that the site-specific variables below are
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
# For `gcc', uncomment the next line.
# CC = gcc

# INCDIR is the parent of the directory that contains the WFDB include (*.h)
# files.  (These files should have been installed in $(INCDIR)/wfdb;  see
# ../lib/Makefile.)
INCDIR = /usr/include

# If you trust your C compiler's optimizer, you may wish to add -O to CFLAGS;
# for debugging, add -g to CFLAGS.  (If you use `gcc', you may use both;  most
# other C compilers do not permit you to do so.)
CFLAGS = -O -I$(INCDIR)

# LIBDIR is the directory that contains the WFDB library (see ../lib/Makefile).
LIBDIR = /usr

# LDFLAGS is appended to the C compiler command line to specify loading the
# WFDB library.  Unless you have changed the value of WFDBLIB in the `Makefile'
# for the WFDB library, `-lwfdb' should be correct.
LDFLAGS = -L$(LIBDIR) -lwfdb

# BINDIR specifies the directory in which the applications will be installed;
# it should be a directory in the PATH of those who will use the applications.
# You will need to have write permission in BINDIR.
BINDIR = /usr/local/bin

# PRINT is the name of the program used to produce listings (including any
# options for the desired formatting).
PRINT = lpr

# SETXPERMISSIONS is the command needed to make the installed files accessible
# to those who will use them.  The value given below makes them readable and
# executable by everyone, and writeable by the owner only.  (If you perform the
# installation as `root', `root' is the owner of the installed files.)
SETXPERMISSIONS = chmod 755

# STRIP is the command used to compact the compiled binaries by removing their
# symbol tables.
STRIP = strip
# To retain the symbol tables for debugging, comment out the previous line, and
# uncomment the next line.
# STRIP = :

# It should not be necessary to modify anything below this line.
# -----------------------------------------------------------------------------

CFILES = a2m.c ad2m.c m2a.c md2a.c readid.c makeid.c edf2mit.c revise.c
XFILES = a2m ad2m m2a md2a readid makeid edf2mit revise
MFILES = Makefile makefile.dos

# General rule for compiling C sources into executable files.  This is
# redundant for most versions of `make', but at least one System V version
# needs it.
.c:
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# `make' or `make install':  build and install applications, clean up
install:	all
	cp $(XFILES) $(BINDIR)
	cd $(BINDIR); $(SETXPERMISSIONS) $(XFILES)
	$(MAKE) clean

# `make all': build applications
all:	$(XFILES)
	$(STRIP) $(XFILES)

# `make clean':  remove intermediate and backup files
clean:
	rm -f $(XFILES) *.o *~

# `make listing': print a listing of WFDB format-conversion application sources
listing:
	$(PRINT) README $(MFILES) $(CFILES)

revise:		revise.c
	$(CC) $(CFLAGS) -I../lib revise.c -o revise $(LDFLAGS)
