# file: Makefile	G. Moody	2 November 1989
#			Last revised:	  7 May 1999
# UNIX 'make' description file for WFDB example programs
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

# If you trust your C compiler's optimizer, you may wish to add -O to CFLAGS;
# for debugging, add -g to CFLAGS.  (If you use `gcc', you may use both;  most
# other C compilers do not permit you to do so.)
CFLAGS = -O

# LDFLAGS is appended to the C compiler command line to specify loading the
# DB library.  Unless you have changed the value of WFDBLIB in the `Makefile'
# for the WFDB library, `-lwfdb' should be correct.
LDFLAGS = -lwfdb

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
