# file: Makefile	G. Moody	9 December 1991
#			Last revised:	  5 May 1999
#
# -----------------------------------------------------------------------------
# `make' description file for PSD estimation programs
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
# This file is used with the UNIX `make' command to install the power spectral
# density (PSD) estimation applications that come with the WFDB Software
# Package.  Before using it for the first time, check that the site-specific
# variables below are appropriate for your system.  To build and install these
# applications and their man pages, just type `make' (from within this
# directory).

# Site-specific variables
# -----------------------

# CC is the name of your C compiler.  These applications can be compiled with
# K&R C compilers (`cc' on most if not all UNIX systems) as well as with ANSI
# C compilers such as GNU C (`gcc').  To use the K&R C compiler available on
# most UNIX systems, uncomment the next line.
CC = cc
# For `gcc', uncomment the next line.
# CC = gcc

# If you trust your C compiler's optimizer, you may wish to add -O to CFLAGS;
# for debugging, add -g to CFLAGS.  (If you use `gcc', you may use both;  most
# other C compilers do not permit you to do so.)
CFLAGS = -O

# BINDIR specifies the directory in which the applications will be installed;
# it should be a directory in the PATH of those who will use the applications.
# You will need to have write permission in BINDIR.
BINDIR = /usr/local/bin

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

# Programs to be compiled.
XFILES = coherence fft log10 lomb memse

# Shell scripts to be installed.
SCRIPTS = hrfft hrlomb hrmem hrplot plot2d plot3d

# `make' or `make install':  build and install applications, clean up
install:	all
	cp $(XFILES) $(SCRIPTS) $(BINDIR)
	cd $(BINDIR); $(SETXPERMISSIONS) $(XFILES) $(SCRIPTS)
	$(MAKE) clean

# `make all': build applications
all:	$(XFILES)
	$(STRIP) $(XFILES)

coherence:	coherence.c
	$(CC) -o coherence -O coherence.c -lm

fft:		fft.c
	$(CC) -o fft -O fft.c -lm

log10:		log10.c
	$(CC) -o log10 -O log10.c -lm

lomb:		lomb.c
	$(CC) -o lomb -O lomb.c -lm

memse:		memse.c
	$(CC) -o memse -O memse.c -lm

# `make clean': remove intermediate and backup files.
clean:
	rm -f *.o *~ $(XFILES)
