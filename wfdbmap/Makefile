# file: Makefile	G. Moody	30 July 2010
#
# To build and install the software in this directory, first install the
# WFDB and plt software packages from PhysioNet, ImageMagick (from
# www.imagemagick.org), then return to this directory and type 'make'.

# The software will be installed in BINDIR.  Make sure BINDIR is in your
# PATH and that you have write permission there.
BINDIR=/usr/local/bin

install:	wfdbmap
	mv wfdbmap $(BINDIR)
	cp -p map-record $(BINDIR)

wfdbmap:	wfdbmap.c signal-colors.h
	gcc -o wfdbmap -O wfdbmap.c -lwfdb

clean:
	rm -f *~
