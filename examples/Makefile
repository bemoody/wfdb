# file: Makefile	G. Moody	2 November 1989
#			Last revised:	4 September 1999
# UNIX 'make' description file for WFDB example programs
#
# -----------------------------------------------------------------------------
# WFDB example programs: how to use the WFDB library
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
# This file is used with the UNIX `make' command to compile the example
# programs from the WFDB Programmer's Guide, for which the sources can be
# found in this directory.  Since these programs are intended for instruction
# rather than `production' use, this `makefile' does not include a procedure
# for installing them.  To compile the examples, just type `make' (from within
# this directory);  the executable files will be left in this directory.  Type
# `make clean' to remove them.

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
LIBDIR = /usr/lib

# LDFLAGS is appended to the C compiler command line to specify loading the
# DB library.  Unless you have changed the value of WFDBLIB in the `Makefile'
# for the WFDB library, `-lwfdb' should be correct.
LDFLAGS = -L$(LIBDIR) -lwfdb

# PRINT is the name of the program used to produce listings (including any
# options for the desired formatting).
PRINT = lpr

# It should not be necessary to modify anything below this line.
# -----------------------------------------------------------------------------

CFILES = psamples.c exgetvec.c exputvec.c exannstr.c example1.c example2.c \
 example3.c example4.c example5.c example6.c example7.c example8.c example9.c \
 example10.c refhr.c
XFILES = psamples exgetvec exputvec exannstr example1 example2 example3 \
 example4 example5 example6 example7 example8 example9 example10 refhr
MFILES = Makefile makefile.dos

# General rule for compiling C sources into executable files.  This is
# redundant for most versions of `make', but at least one System V version
# needs it.
.c:
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# `make' or `make compile':  compile the examples
compile:	$(XFILES)

# `make listing':  print a listing of sources
listing:
	$(PRINT) README $(MFILES) $(CFILES)

# `make clean':  remove executable, intermediate and backup files
clean:
	rm -f $(XFILES) *.o *~ core

# Odds and ends.
example10.c:
	mv exampl10.c example10.c
