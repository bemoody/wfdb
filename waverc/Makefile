# file: Makefile	G. Moody	 20 October 1996
#
# `make' description file for compiling remote-control applications for WAVE

# CC is the name of your C compiler.  These applications can be compiled with
# K&R C compilers (`cc' on most if not all UNIX systems) as well as with ANSI
# C compilers such as GNU C (`gcc').  To use the K&R C compiler available on
# most UNIX systems, uncomment the next line.
CC = cc
# For `gcc', uncomment the next line.
# CC = gcc

# BINDIR specifies the directory in which the applications will be installed;
# it should be a directory in the PATH of those who will use the applications.
# You will need to have write permission in BINDIR.  Users of this software
# will need to have search (execute) permission in BINDIR.
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
	$(CC) -o wavescript -O wavescript.c

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
