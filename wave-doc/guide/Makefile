# file: Makefile	G. Moody	 9 August 1992
#			Last revised:	4 February 2000
# `make' description file for printing the WAVE User's Guide

# Copyright(C) Massachusetts Institute of Technology 2000. All rights reserved.

# Type `make guide' to print the compressed PostScript source.  You will need
# gzip, a PostScript printer (or GhostScript and a supported printer), and
# about 130 sheets of paper.  Make sure that PRINT is set correctly for your
# system below.

# If you have LaTeX2e, makeindex, and dvips, you can regenerate the PostScript
# from the text source and the figure files.  To do this, type `make wug.psz'.
# If you have an older version of LaTeX (2.09 or earlier), you will need to
# update it to LaTeX2e in order to do this successfully.

# If you have LaTeX2html and additional freely available software described
# in the LaTeX2html documentation, you can regenerate the HTML version of the
# WAVE User's Guide by typing `make html' (see below).

# All of the software mentioned above is freely available;  see ../SOURCES.

# TROFF is the name of the program that prints UNIX troff files (needed to
# format the covers of the guide).  Use `groff' if you have GNU groff (the
# preferred formatter).
TROFF = groff
# Use `ptroff' if you have Adobe TranScript software.
# TROFF = ptroff
# Consult your system administrator if you have neither `groff' nor `ptroff'.
# Other (untested) possibilities are `psroff', `ditroff', `nroff', and `troff'.

# PRINT is the command for printing PostScript files.  If your printer is not
# a PostScript printer, see the GhostScript documentation to see how to do
# this (since the figure files are in PostScript form, it is not sufficient
# to use a non-PostScript dvi translator such as dvilj).
PRINT = lpr

explain:
	@echo "Type 'make guide' to print the WAVE User's Guide."

guide:		wug.psz wugcover.psz
	gzip -d <wugcover.psz | $(PRINT)
	gzip -d <wug.psz | $(PRINT)

# `make wug.psz':  regenerate compressed PostScript from the TeX dvi output,
# interpolating the figure files where required.  This is done using dvips
# (included in the TeX distribution) and gzip.

wug.ps:		wug.dvi
	cp -p ../grey/*.ps .
	dvips -o wug.ps wug

wug.psz:	wug.ps
	gzip <wug.ps >wug.psz

# `make wug.dvi':  process the LaTeX source.  The figure files are not included
# in the dvi file, but they must be read by LaTex in order to determine their
# sizes.  LaTeX needs three passes to generate the table of contents, list of
# figures, and index correctly.  makeindex is included in the TeX distribution.

wug.dvi:	wug.tex
	cp -p ../grey/*.ps .
	latex wug
	makeindex wug.idx
	latex wug
	makeindex wug.idx
	latex wug

wugcover.psz:
	$(TROFF) cover.wug | gzip >wugcover.psz

# `make html':  process the LaTeX source using latex2html.  This has been
# tested with latex2html version 96.1.  The file `.latex2html.init' in this
# directory defines special settings needed to create the HTML files as
# distributed with WAVE.  If you are not using Netscape as your browser, you
# may want to select a different HTML version or other optimizations.  The
# output files will be placed in a directory called `wug' (initially within
# this directory, but moved into the `html' directory two levels up from here
# when done), and the root file in the HTML tree will be `wug/wug.html'.
# Cross-references between the WAVE User's Guide, the ECG Database Programmer's
# Guide, and the man pages in the ECG Database Applications Guide, all assume
# that the HTML versions of these guides are found in `dbpg' and `dbag'
# directories at the same level as the `wug' directory;  see ../../doc/Makefile
# to see how this is done.
#
# Note that (at least with version 96.1 of LaTeX2html) this process can be
# s l o w . . . (25 minutes on a 100 MHz Pentium with 32 Mb of RAM).  It may
# not work at all without a generous amount of RAM and swap space.  It's
# probably not a good idea to try this while Netscape is running, for example.
#
# Note that the latex2html 96.1 equivalent of `makeindex' leaves many duplicate
# entries in the index.  `fixindex' (a shell script in this directory) removes
# most of them.  If you use a later version of latex2html, `fixindex' may not
# be necessary.

HTMLDIR = ../../../html

html:	wug.ps 	# (we also need wug.aux, made with wug.dvi by 'make wug.ps')
	-mkdir $(HTMLDIR)
	-mkdir $(HTMLDIR)/wug
	cp -f wug.ps $(HTMLDIR)/wug/wug.ps
	cp -f icons/*.png *.png stdev.c example.xws $(HTMLDIR)/wug
	-mkdir $(HTMLDIR)/dbag
	../../doc/manhtml.sh $(HTMLDIR)/dbag xview.7
	cp -f ../grey/*.ps .         # restore greyscale figures
	cp -f ../color/*.ps .        # use any available color figures
	latex2html -dir $(HTMLDIR)/wug -local_icons wug
	./fixindex $(HTMLDIR)/wug    # optional; see above
	cp dossify-html fixlinks $(HTMLDIR)/wug
	cd $(HTMLDIR)/wug; ./dossify-html *.html
	cd $(HTMLDIR)/wug; rm -f dossify-html fixlinks *.html *.orig
	cd $(HTMLDIR)/wug; rm -f .ID_MAP .IMG_PARAMS .ORIG_MAP images.*
	mv $(HTMLDIR)/wug/*.pl .
	cd $(HTMLDIR)/wug; ln -s wug.htm index.html; find `pwd` -print | doschk
	./fixinfo >../../wave/wave.info
	cp -p ../../wave/wave.info /usr/local/help/wave/wave.info

clean:
	rm -f wug.aux wug.dvi wug.idx wug.ilg wug.ind wug.lof wug.log wug.toc
	rm -f wug.lot wug.psz *.ps wugcover.psz internals.pl labels.pl stdev *~

# `make stdev':  This is an example program used in chapter 3 of the WAVE
# User's Guide.
stdev:		stdev.c
	cc -o stdev -I/usr/local/include -O stdev.c -L/usr/local/lib -ldb
