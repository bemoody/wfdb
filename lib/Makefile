# file: Makefile	G. Moody	 6 July 1983
#			Last revised:  6 January 2000		Version 10.1.0
# UNIX 'make' description file for compiling the WFDB library with NETFILES
#
# _____________________________________________________________________________
# wfdb: a library for reading and writing annotated waveforms(time series data)
# Copyright (C) 2000 George B. Moody
#
# This library is free software; you can redistribute it and/or modify it under
# the terms of the GNU Library General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for
# more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# You may contact the author by e-mail (george@mit.edu) or postal mail
# (MIT Room E25-505A, Cambridge, MA 02139 USA).  For updates to this software,
# please visit PhysioNet (http://www.physionet.org/).
# _____________________________________________________________________________
#
# This file is used with the UNIX `make' command to install the WFDB library.
# Before using it for the first time, check that the site-specific variables
# below are appropriate for your system.  To build and install the standard
# WFDB library, just type `make' (from within this directory).  To build and
# install the alternate WFDB library (not available on all systems), type
# `make slib'.

# Note:  This version contains experimental NETFILES support, which requires
# that you have previously installed libwww (see http://www.w3.org/Library/);
# if not, you must change the definitions of SCFLAGS and SBUILDLIB below.
# As is, this Makefile is preconfigured for Linux and requires minor changes
# for other supported operating systems.  It builds a static version of the
# library without NETFILES support, and a shared (dynamically loadable) version
# of the library with NETFILES support.

# NETFILES support is known to work well under Linux (with both 2.0 and 2.2
# series kernels and glibc 2.0 and 2.1) provided that the library is compiled
# as a shared (dynamically loadable) library.  The library does not compile
# properly as a static library under Linux if NETFILES is enabled (apparently
# because of redefinitions in the libwww include files).

# For further information about NETFILES, see README.NETFILES in the top-level
# directory of the WFDB software package.

# The WFDB library functions assume that a "char" is 8 bits, a "short int" is
# 16 bits, an "int" is at least 16 bits, and a "long" is at least 32 bits.  The
# last two assumptions are valid for ANSI C compilers, and for almost all older
# C compilers as well.  The assumption most likely to be incorrect is that
# regarding the "short int";  if it is not true, it may be necessary to rewrite
# wfdb_g16() in `wfdbio.c' and r16() in `signal.c'.

# Version numbers
# ---------------

# Each release of the WFDB Software Package is identified by a three-part
# version number, defined in this section (unless overridden by settings in the
# top-level Makefile for the WFDB Software Package).  See the top-level
# Makefile (in the parent of this directory) for details.
MAJOR = 10
MINOR = 1
RELEASE = 0
VERSION = $(MAJOR).$(MINOR).$(RELEASE)

# Historical note:  Versions earlier than 10.0.0, released between 1981 and
# 1998, were known as the DB library.  The name was changed in 1999 for
# version 10.0.0 because of the proliferation of another, more general-purpose,
# DB library (the BSD version of the UNIX dbm library), with which this library
# should not be confused.

# Site-specific variables
# -----------------------

# CC is the name of your C compiler.  The WFDB library can be compiled with K&R
# C compilers (`cc' on most if not all UNIX systems) as well as with ANSI C
# compilers such as GNU C (`gcc').  To use the vendor-supplied C compiler
# available on most UNIX systems, uncomment the next line.
CC = cc
# For `gcc', uncomment the next line (and see CFLAGS and SCFLAGS below).
# CC = gcc
# On systems such as Linux where `gcc' is the standard C compiler, `gcc' may
# also be known as `cc'; in such cases, it doesn't matter if CC is cc or gcc.

# INCDIR is a directory searched by CC for #include <...> files.
# Add -I$(INCDIR) to CFLAGS (below) if you change INCDIR.
INCDIR = /usr/include

# CCDEFS is the set of C compiler options needed to set preprocessor variables
# while compiling the WFDB Software Package.  You should include definitions of
# the major, minor, and release numbers, as shown below.  Add the following
# options to CCDEFS as appropriate (separating them by spaces if you use more
# than one):
#   -DISPRINTF	  if you do not have `stdlib.h' and your `sprintf' returns an
#		   int (see wfdblib.h)
#   -DNETFILES	  if you have libwww and want to be able to access WFDB files
#		   remotely via http or ftp (as well as local files)
#   -DNOMALLOC_H  if you have neither `stdlib.h' nor `malloc.h' (see wfdblib.h)
#   -DNOSTRTOK	  if your standard C library does not include function `strtok'
#		   (see wfdbio.c)
#   -DNOTIME	  if you do not have `time.h' (see signal.c)
#   -DOLDC	  if you have neither `stdarg.h' nor `varargs.h' (see wfdbio.c)
#   -DSOLARIS     if you are using Solaris #include <...> files (see wfdb.h)
# If your C compiler fails to compile `signal.c', add -DBROKEN_CC to CCDEFS
# and try again (see signal.c).
CCDEFS = -DWFDB_MAJOR=$(MAJOR) -DWFDB_MINOR=$(MINOR) -DWFDB_RELEASE=$(RELEASE)

# CFLAGS is the set of C compiler options.  CFLAGS should always include
# CCDEFS.  Add the following options to CFLAGS as appropriate (separating them
# by spaces if you use more than one):
#   -g		  to save symbols for debugging
#   -O		  if you trust your C compiler's optimizer
#   -I$(INCDIR)   if you have changed the value of INCDIR (above)
# With the exception of `gcc', most C compilers do not allow you to use -g and
# -O simultaneously.
CFLAGS = -g $(CCDEFS)
# `gcc' users may comment out the previous line, and uncomment the next one.
# CFLAGS = -g -O $(CCDEFS)
# Note: If your operating system supports shared (also known as dynamically
# loaded or run-time linked) libraries, as well as conventional static
# (compile-time linked) libraries, a good strategy is to compile a shared
# library with optimization (-O) and a static library with debugging symbols
# (-g).  The default in most cases is for your program to be linked to shared
# libraries if they are available, so this approach gives you the benefit of
# optimization in the WFDB library by default.  In order to use a debugger with
# your program, however, you will probably need to compile an unoptimized
# version of it (unless you are using `gcc'), with debugging symbols, and link
# it to static libraries (unless you are debugging with `gdb'); so in this case
# it won't hurt (and may be useful) to have debugging symbols for the WFDB
# library as well.

# LIBDIR should be one of the directories searched by CC's loader for -l...
# libraries.  You will also need to have write permission in LIBDIR.  If you
# plan to make a shared WFDB library (see below), note that WFDB application
# users will need to have read permission in LIBDIR.
LIBDIR = /usr/lib

# WFDBLIB is the name of the standard WFDB library.  In order to access it via
# `-lwfdb', WFDBLIB should be `libwfdb.a'.
WFDBLIB = libwfdb.a

# If your system requires indexed libraries, uncomment the next two lines.
LFILES = $(OFILES)
RANLIB = ranlib
# Otherwise, if your system requires ordered libraries, uncomment the next two
# lines.  (As distributed, this file contains a sorted list of OFILES already.)
# LFILES = `lorder $(OFILES) | tsort`
# RANLIB = :
# Otherwise, uncomment the next two lines.
# LFILES = $(OFILES)
# RANLIB = :

# AR is the name of the command used to build a standard library from `.o'
# files, and ARFLAGS is the set of options used when doing so.  AR and ARFLAGS
# are usually `make' builtins and need not be defined here;  under HP UX, and
# perhaps other versions of UNIX, however, you will need to define these
# variables by uncommenting the next two lines.
# AR = ar
# ARFLAGS = rv

# BUILDLIB is the command to be used to create WFDBLIB once all of its
# components have been compiled separately.  This will not usually need to be
# changed.
BUILDLIB = $(AR) $(ARFLAGS) $(WFDBLIB) $(LFILES)

# PRINT is the name of the program used to produce listings (including any
# options for the desired formatting).
PRINT = lpr

# SETDPERMISSIONS is similarly used to make directories created during the
# installation accessible.
SETDPERMISSIONS = chmod 755

# SETPERMISSIONS is the command needed to make the installed files accessible
# to those who will use them.  The value given below makes them readable by
# everyone, and writeable by the owner only.  (If you perform the installation
# as `root', `root' is the owner of the installed files.)
SETPERMISSIONS = chmod 644

#.............................................................................

# Ignore this section of site-specific variables unless your system supports
# two or more types of libraries.  As distributed, this section contains
# settings suitable for generating an ELF-format shared library under Linux.
# By selecting alternative settings below, you can make a shared library with
# `cc' under SunOS or Solaris, `gcc' version 2.0 or later (tested under SunOS,
# Solaris, Linux, and HP/UX), or with `cc' or `gcc' under UNIX SVR4 (not
# tested).  By modifying these settings, this mechanism can be used to generate
# other types of variant libraries (e.g., for alternative memory models on
# systems that support more than one, or using an alternative C compiler or
# cross-compiler).

# SCC is the C compiler to be used for the alternate library.
SCC = $(CC)

# SINCDIR is a directory searched by SCC for include <...> files.
# Add -I$(SINCDIR) to SCFLAGS (below) if you change SINCDIR.
SINCDIR = $(INCDIR)

# SCFLAGS is the set of C compiler options used when compiling the alternate
# library.  For SunOS `cc', uncomment the next line.
# SCFLAGS = -pic -O $(CCDEFS)
# For Solaris or SVR4 `cc', uncomment the next line.
# SCFLAGS = -K PIC -O $(CCDEFS)
# For `gcc', uncomment the next line.
# SCFLAGS = -fpic -g -O $(CCDEFS)
# For `gcc' with NETFILES, uncomment the next line.
SCFLAGS = -fpic -g -O $(CCDEFS) -DNETFILES `libwww-config --cflags`
# See comments on CFLAGS (above).  If you don't have `gdb', you won't be able
# to debug the shared library, so you might want to remove the `-g' option in
# that case.

# SLIBDIR is the directory in which the alternate library is to be installed.
SLIBDIR = $(LIBDIR)

# SWFDBLIB_BASENAME is the name, without version numbers, of the alternate
# library.  SWFDBLIB_SONAME is the shared object name ("soname") of the
# alternate library; normally, this includes the base name and the major
# version number only. SWFDBLIB is the complete name, including the minor
# version number.  Symbolic links from SWFDBLIB to SWFDBLIB_BASENAME and
# SWFDBLIB_SONAME will be created as the final step in building the alternate
# library.  If you are building a shared library, these links are necessary;
# otherwise they are not required.
SWFDBLIB_BASENAME = libwfdb.so
SWFDBLIB_SONAME = $(SWFDBLIB_BASENAME).$(MAJOR)
SWFDBLIB = $(SWFDBLIB_SONAME).$(MINOR)
# For a shared library, the soname is significant for proper run-time binding.
# If you change function interfaces in the library, change its soname by
# incrementing the major version number;  when you do this, set the minor
# version number to zero.  If you change the library without changing the
# function interfaces, increment the minor version number; this allows existing
# binaries to use the new version without recompilation, since the soname is
# unchanged in this case.

# SLIBOBJS is the list of *.o modules to be included in the alternate library.
# This should match OFILES below, and should not need to be changed.
SLIBOBJS = wfdbinit.o annot.o signal.o calib.o wfdbio.o

# SBUILDLIB is the command that creates the alternate library once its
# components have been compiled separately.  For SunOS 4.x, using either `cc'
# or `gcc', uncomment the next line.
# SBUILDLIB = ld -o $(SWFDBLIB) -assert pure-text $(SLIBOBJS)
# For Solaris, uncomment the next line instead.
# SBUILDLIB = ld -o $(SWFDBLIB) -G -z text $(SLIBOBJS)
# For SVR4, uncomment the next line instead.
# SBUILDLIB = ld -o $(SWFDBLIB) -z text $(SLIBOBJS)
# For (at least some versions of) HP/UX, uncomment the next line instead.
# SBUILDLIB = ld -b -o $(SWFDBLIB) $(SLIBOBJS)
# For Linux without NETFILES (see above), uncomment the next line instead.
# SBUILDLIB = gcc -shared -Wl,-soname,$(SWFDBLIB_SONAME) -o $(SWFDBLIB) \
#  $(SLIBOBJS)
# For Linux with NETFILES (see above), add -DNETFILES to CCDEFS and uncomment
# the next line instead.
SBUILDLIB = gcc -shared -Wl,-soname,$(SWFDBLIB_SONAME) \
  `libwww-config --libs` -o $(SWFDBLIB) $(SLIBOBJS)

# SRANLIB is the name of the program used to index the alternate library.
# Shared libraries are not generally indexed.
SRANLIB = :

# LDCONFIG is the name of the program needed to refresh the system's cached
# index of shared libraries (needed under SunOS and Linux).
# Under SunOS, use /usr/etc/ldconfig.
# LDCONFIG = /usr/etc/ldconfig $(SLIBDIR)
# Under Linux, use /sbin/ldconfig.
LDCONFIG = /sbin/ldconfig
# Under Solaris, SVR4, and other operating systems that do not maintain a
# cached index of shared libraries, uncomment the next line.
# LDCONFIG = :

# SMAKE is the `make' command that is to be executed when `make slib' is run.
SMAKE = $(MAKE) CC=$(SCC) "CFLAGS=$(SCFLAGS)" INCDIR=$(SINCDIR) \
 LIBDIR=$(SLIBDIR) WFDBLIB=$(SWFDBLIB) "BUILDLIB=$(SBUILDLIB)" \
 RANLIB=$(SRANLIB) "LDCONFIG=$(LDCONFIG)"

# It should not be necessary to modify anything below this line.
# -----------------------------------------------------------------------------

INCLUDES = $(INCDIR)/wfdb/wfdb.h $(INCDIR)/wfdb/ecgcodes.h \
 $(INCDIR)/wfdb/ecgmap.h
COMPAT_INCLUDES = $(INCDIR)/ecg/db.h $(INCDIR)/ecg/ecgcodes.h \
 $(INCDIR)/ecg/ecgmap.h
HFILES = wfdb.h ecgcodes.h ecgmap.h wfdblib.h
CFILES = wfdbinit.c annot.c signal.c calib.c wfdbio.c
OFILES = wfdbinit.o annot.o signal.o calib.o wfdbio.o
MFILES = Makefile makefile.dos

# `make' or `make install':  build and install the WFDB library and includes
install:	$(LIBDIR)/$(WFDBLIB)
$(LIBDIR)/$(WFDBLIB):	$(INCDIR) $(LIBDIR) $(INCLUDES) $(OFILES)
	$(BUILDLIB)
	cp $(WFDBLIB) $(LIBDIR) 
	$(SETPERMISSIONS) $(LIBDIR)/$(WFDBLIB)
	$(RANLIB) $(LIBDIR)/$(WFDBLIB)

# `make compat':  install the includes needed for source compatibility with
# applications written for pre-version 10.0.0 versions of this library
compat:		$(INCLUDES) $(COMPAT_INCLUDES)

# `make lib': build the library in the current directory
lib:		$(OFILES)
	$(BUILDLIB)
	$(RANLIB) $(WFDBLIB)

# `make slib-test':  build the alternate WFDB library in the current directory
slib-test:
	$(MAKE) tidy
	$(SMAKE) lib
	$(MAKE) tidy

# `make slib':  build and install the alternate WFDB library
slib:
	$(MAKE) tidy
	$(SMAKE)
	$(MAKE) tidy
	cd $(SLIBDIR); ln -sf $(SWFDBLIB) $(SWFDBLIB_BASENAME)
	cd $(SLIBDIR); ln -sf $(SWFDBLIB) $(SWFDBLIB_SONAME)
	@$(LDCONFIG) || echo Warning: "$(LDCONFIG)" was unsuccessful

# `make tidy':  remove intermediate and backup files
tidy:
	rm -f $(OFILES) *~

# `make clean':  also remove previously compiled versions of the library
clean:
	rm -f $(WFDBLIB) $(SWFDBLIB) $(OFILES) *~

# `make TAGS':  make an `emacs' TAGS file
TAGS:		$(HFILES) $(CFILES)
	@etags $(HFILES) $(CFILES)

# `make listing':  print a listing of WFDB library sources
listing:
	$(PRINT) README $(MFILES) $(HFILES) $(CFILES)

# Rules for creating installation directories
$(INCDIR):
	-mkdir $(INCDIR)
	-$(SETDPERMISSIONS) $(INCDIR)
$(LIBDIR):
	-mkdir $(LIBDIR)
	-$(SETDPERMISSIONS) $(LIBDIR)

# Rules for installing the include files
$(INCDIR)/wfdb:	$(INCDIR)
	-mkdir $(INCDIR)/wfdb
	-$(SETDPERMISSIONS) $(INCDIR)/wfdb
$(INCDIR)/wfdb/wfdb.h:		$(INCDIR)/wfdb wfdb.h
	cp -p wfdb.h $(INCDIR)/wfdb; $(SETPERMISSIONS) $(INCDIR)/wfdb/wfdb.h
$(INCDIR)/wfdb/ecgcodes.h:	$(INCDIR)/wfdb ecgcodes.h
	cp -p ecgcodes.h $(INCDIR)/wfdb
	$(SETPERMISSIONS) $(INCDIR)/wfdb/ecgcodes.h
$(INCDIR)/wfdb/ecgmap.h:		$(INCDIR)/wfdb ecgmap.h
	cp -p ecgmap.h $(INCDIR)/wfdb
	$(SETPERMISSIONS) $(INCDIR)/wfdb/ecgmap.h

# Rules for installing the compatibility (pre-10.0.0) include files
$(INCDIR)/ecg:	$(INCDIR)
	-mkdir $(INCDIR)/ecg
	-$(SETDPERMISSIONS) $(INCDIR)/ecg
$(INCDIR)/ecg/db.h:		$(INCDIR)/ecg db.h
	cp -p db.h $(INCDIR)/ecg; $(SETPERMISSIONS) $(INCDIR)/ecg/db.h
$(INCDIR)/ecg/ecgcodes.h:	$(INCDIR)/ecg $(INCDIR)/wfdb/ecgcodes.h
	ln -s $(INCDIR)/wfdb/ecgcodes.h $(INCDIR)/ecg/ecgcodes.h
$(INCDIR)/ecg/ecgmap.h:		$(INCDIR)/ecg $(INCDIR)/wfdb/ecgmap.h
	ln -s $(INCDIR)/wfdb/ecgmap.h $(INCDIR)/ecg/ecgmap.h

# Prerequisites for the library modules
wfdbinit.o:	wfdb.h wfdblib.h wfdbinit.c
annot.o:	wfdb.h ecgcodes.h ecgmap.h wfdblib.h annot.c
signal.o:	wfdb.h wfdblib.h signal.c
calib.o:	wfdb.h wfdblib.h calib.c
wfdbio.o:	wfdb.h wfdblib.h wfdbio.c
