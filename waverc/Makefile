# file: Makefile	G. Moody	 20 October 1996
#			Last revised:	 16 October 1999
# UNIX 'make' description file for remote-control applications for WAVE
#
# -----------------------------------------------------------------------------
# waverc: remote-control applications for WAVE
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
# CC is the name of your C compiler.  These applications can be compiled with
# K&R C compilers (`cc' on most if not all UNIX systems) as well as with ANSI
# C compilers such as GNU C (`gcc').  To use the K&R C compiler available on
# most UNIX systems, uncomment the next line.
CC = cc
# For `gcc', uncomment the next line.
# CC = gcc

# BINDIR specifies the directory in which both WAVE and these applications will
# be installed;  it should be a directory in the PATH of those who will use the
# applications.  You will need to have write permission in BINDIR.  Users of
# this software will need to have search (execute) permission in BINDIR.
BINDIR = /usr/local/bin

# URLV is the command that starts your web browser if necessary and opens the
# URL named in its first argument.  The following works properly with Netscape
# 1.1 and later versions;  if you are using a different browser, consult its
# documentation.
URLV='( netscape -remote "openURL($$1)" 2>/dev/null || netscape $$1 ) &'

# `make install' installs `wavescript' and `wave-remote'.  See the WAVE User's
# Guide for instructions on setting up `wavescript' as a helper application for
# your Web browser.
install:	urlview wavescript wave-remote
	strip wavescript
	strip wave-remote
	cp urlview wavescript wave-remote $(BINDIR)
	chmod 755 $(BINDIR)/urlview $(BINDIR)/wavescript $(BINDIR)/wave-remote

# `make all' creates urlview, wavescript, and wave-remote without installing
# them.
all:	wavescript wave-remote

# `urlview' opens a web browser to view a named URL.
urlview:	Makefile
	cp urlvhead urlview
	echo $(URLV) >>urlview

# `wavescript' reads commands from a named file and passes them to WAVE.
wavescript:	wavescript.c
	$(CC) -o wavescript -DBINDIR=$(BINDIR) -O wavescript.c

# `wave-remote' passes its command-line arguments as commands to WAVE.
wave-remote:	wave-remote.c
	$(CC) -o wave-remote -O wave-remote.c

# `wave-remote-test' looks like WAVE to `wavescript' and `wave-remote', and
# can be used to verify their proper operation.  Start `wave-remote-test'
# before starting `wavescript' or `wave-remote';  the commands these programs
# send to WAVE should appear on the standard output of `wave-remote-test.
wave-remote-test:	wave-remote-test.c
	$(CC) -o wave-remote-test -O wave-remote-test.c

# `make clean':  remove intermediate and backup files
clean:
	rm -f urlview wavescript wave-remote wave-remote-test *~
