# file: Makefile	G. Moody	5 September 1990
#			Last revised:    11 January 2000	Version 10.1.1
# UNIX 'make' description file for the WFDB software package
#
# -----------------------------------------------------------------------------
# WFDB software for creating & using annotated waveform (time series) databases
# Copyright (C) 2000 George B. Moody
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
# In addition, you can redistribute and/or modify the WFDB library (contained
# within the 'lib' directory) under the terms of the GNU Library General
# Public License as published by the Free Software Foundation; either version
# 2 of the License, or (at your option) any later version.  For details, see
# 'lib/COPYING.LIB'.
#
# You may contact the author by e-mail (george@mit.edu) or postal mail
# (MIT Room E25-505A, Cambridge, MA 02139 USA).  For updates to this software,
# please visit PhysioNet (http://www.physionet.org/).
# _____________________________________________________________________________
#
# This file is used with the UNIX `make' command to compile, install, and
# create archives of the WFDB software package.  Before using it
# for the first time, check that the site-specific variables below are
# appropriate for your system.  To compile the package, just type `make' (from
# within this directory).  To install the package, type `make install'.  To
# create source archives, type `make tarballs';  or to make a binary archive,
# type `make bin-tarball'.  Making archives requires PGP, gzip, and GNU tar).

# ARCH specifies the type of CPU and the operating system (e.g., 'i686-Linux').
# This symbol is used only to generate a name for the binary archive;  it
# does not affect how the software is compiled.
ARCH=`uname -m`-`uname -s`

# WFDBROOT specifies the common root of the directories where the installed
# files go.  Reasonable choices are /usr, /usr/local, /opt, or $(HOME).  If
# WFDBROOT is not /usr, you will probably need to specify -I and -L options
# when compiling your own WFDB applications.
WFDBROOT = /usr

# WFDBDIRS specifies the directories where the installed files go.  Within
# WFDBDIRS, these symbols are defined for use in nested Makefiles:
# INSTALL	used as a synonym for WFDBDIRS in nested Makefiles
#  BINDIR	applications go here
#   DBDIR	calibration and samples of waveform data go here
# HELPDIR	help files (mostly for WAVE) go here
#  INCDIR	library includes go in $(INCDIR)/wfdb
#  LIBDIR	library itself goes here
# MENUDIR	default WAVE menu files go here
#  PSPDIR	PostScript prolog files go here
#  RESDIR	X11 resource files go here
WFDBDIRS = \
  INSTALL=$(WFDBROOT) \
   BINDIR=$(WFDBROOT)/bin \
    DBDIR=$(WFDBROOT)/database \
  HELPDIR=$(WFDBROOT)/help \
   INCDIR=$(WFDBROOT)/include \
   LIBDIR=$(WFDBROOT)/lib \
  MENUDIR=$(WFDBROOT)/lib \
   PSPDIR=$(WFDBROOT)/lib/ps \
   RESDIR=$(WFDBROOT)/lib/X11/app-defaults

# 'make' or 'make install': compile and install the WFDB software package
install:	install-slib
	cd wave;     $(MAKE) $(WFDBDIRS)
	cd waverc;   $(MAKE) $(WFDBDIRS)
	cd app;      $(MAKE) $(WFDBDIRS)
	cd psd;      $(MAKE) $(WFDBDIRS)
	cd convert;  $(MAKE) $(WFDBDIRS)
	cd data;     $(MAKE) $(WFDBDIRS)

# 'make install-slib': compile and install the dynamically-linked WFDB library
# and include (*.h) files
install-slib:
	cd lib;      $(MAKE) $(WFDBDIRS) slib

# 'make all': compile the WFDB applications without installing them (requires
# installation of the dynamically-linked WFDB library and includes)
all:		install-slib
	cd wave;     $(MAKE) $(WFDBDIRS) wave
	cd waverc;   $(MAKE) $(WFDBDIRS) all
	cd app;      $(MAKE) $(WFDBDIRS) all
	cd psd;      $(MAKE) $(WFDBDIRS) all
	cd examples; $(MAKE) $(WFDBDIRS) compile
	cd convert;  $(MAKE) $(WFDBDIRS) all

# 'make clean': remove binaries, other cruft from source directories
clean:
	cd app;      $(MAKE) clean
	cd convert;  $(MAKE) clean
	cd data;     $(MAKE) clean
	cd doc;      $(MAKE) clean
	cd examples; $(MAKE) clean
	cd fortran;  $(MAKE) clean
	cd lib;      $(MAKE) clean
	cd psd;      $(MAKE) clean
	cd wave;     $(MAKE) clean
	cd wave-doc; $(MAKE) clean
	cd waverc;   $(MAKE) clean
	cd wview;    $(MAKE) -f clean
	rm -f *~

# 'make test-lib': compile the dynamically-linked WFDB library without
# installing it
test-lib:
	cd lib;      $(MAKE) $(WFDBDIRS) slib-test

# 'make test' or 'make test-all': compile the WFDB applications without
# installing them (installs the dynamically-linked WFDB library and includes
# into subdirectories of $(HOME)/wfdb-test)
test test-all:
	-mkdir $(HOME)/wfdb-test
	-mkdir $(HOME)/wfdb-test/include
	-mkdir $(HOME)/wfdb-test/lib
	$(MAKE) WFDBROOT=$(HOME)/wfdb-test all

# 'make test-install': compile and install the WFDB software package into
# subdirectories of $(HOME)/wfdb-test
test-install:
	-mkdir $(HOME)/wfdb-test
	-mkdir $(HOME)/wfdb-test/bin
	-mkdir $(HOME)/wfdb-test/database
	-mkdir $(HOME)/wfdb-test/help
	-mkdir $(HOME)/wfdb-test/include
	-mkdir $(HOME)/wfdb-test/lib
	$(MAKE) WFDBROOT=$(HOME)/wfdb-test install

# 'make tarballs': clean up the source directories, then make a pair of gzipped
# tar source archives of the WFDB software package (with and without the
# documentation), and generate PGP signature blocks for the archives
tarballs:	clean
	cd ..; tar --create --file wfdb.tar.gz --verbose --gzip \
                '--exclude=wfdb/*CVS' wfdb
	cd ..; tar --create --file wfdb-no-docs.tar.gz --verbose --gzip \
                '--exclude=wfdb/*CVS' '--exclude=wfdb/*doc' wfdb
	cd ..; pgps -b wfdb.tar.gz wfdb-no-docs.tar.gz

# 'make bin-tarball': make a gzipped tar archive of the WFDB software package
# binaries and other installed files
bin-tarball:	test-install
	cd $(HOME)/wfdb-test;  tar cfvz ../wfdb-$(ARCH).tar.gz .
	cd ..; pgps -b wfdb-$(ARCH).tar.gz
