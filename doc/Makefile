# file: Makefile	G. Moody	24 June 1989
#			Last revised: 14 November 1999
# `make' description file for WFDB software documentation
#
# -----------------------------------------------------------------------------
# WFDB applications: programs for working with annotated signals
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
# This file is used with the UNIX `make' command to install on-line
# documentation for the WFDB Software Package, and also to format and print it.
# Before using this file for the first time, check that the site-specific
# variables below are appropriate for your system.

# The primary documentation for the WFDB Software Package consists of 3 guides:
#  * The WFDB Applications Guide.  This guide documents the application
#    programs provided in the app, convert, psd, wave (briefly), and wview
#    subdirectories of wfdb, as well as the file formats supported by the WFDB
#    Software Package. It also includes a tutorial on evaluation of arrhythmia
#    and ischemia detectors using this software and standard databases.
#  * The WFDB Programmer's Guide.  This guide documents the WFDB library.
#    It contains both tutorial and reference material, with many examples, that
#    will be useful if you wish to develop your own application programs for
#    reading, analyzing, or creating digitized, optionally annotated,
#    recordings of signals.
#  * The WAVE User's Guide.  This guide (found in wfdb/wave-doc, not in this
#    directory) includes a comprehensive tutorial, with reference material,
#    for WAVE, an interactive waveform browser with facilities for annotation
#    editing and control of external analysis programs. WAVE runs on PCs under
#    Linux, and on SPARC-based systems under SunOS or Solaris.
# The files in this directory can be used to prepare printed or on-line
# versions of the first two of these guides.

# On-line documentation
# =====================

# To install the man pages (the text of the WFDB Applications Guide) on-line,
# type `make install' (or just `make').  The individual pages will be
# accessible to the UNIX `man' command, and to similar commands such as `xman',
# provided that MANDIR (see below) is in your MANPATH (type `man man' for
# further information).

# If GNU emacs or GNU info has been installed on your system, a hypertext
# version of the WFDB Programmer's Guide may be installed by typing `make
# info'.  Once the info files have been installed, provided that INFODIR is in
# your INFOPATH (see below), you can browse the guide from within emacs by
# typing C-h i to enter the info system, and selecting `WFDB library' from
# the top-level menu.  Note that `make info' adds an entry for the Guide to the
# top-level `info' file ($INFODIR/dir).  If you install a GNU emacs update,
# this entry may disappear; to restore it, simply `make info' again.

# To install both the WFDB Applications Guide and the WFDB Programmer's Guide
# in HTML form, suitable for reading using a World Wide Web browser, type `make
# html'.  In order to do this successfully, you must have installed
# `latex2html' (to translate the appendices of the Applications Guide), and
# `rman' (to translate the man pages in the Applications Guide).  You also need
# `texi2html' (included here, to translate the WFDB Programmer's Guide),
# and Perl (in order to run `latex2html' and `texi2html').  Links to all of
# these are available from http://ecg.mit.edu, along with the latest versions
# of the files in this directory.

# Printed documentation
# =====================

# Individual man pages or info nodes can be printed using the facilities of
# man and emacs respectively, or (from the HTML versions) using your web
# browser's print facilities.  Complete printed copies of the WFDB
# Applications and Programmer's Guides can be obtained in several ways:
#  - Preformatted versions are provided in ../../html/*.ps.  If you don't have
#    a PostScript printer, you may still be able to use ghostscript to print
#    these versions on your printer (most popular laser, inkjet, and dot
#    matrix printers are supported by ghostscript).
#  - The sources for these manuals are provided in this directory;  you should
#    use these if you have made additions to the manuals or wish to customize
#    them in any other way.  To format the sources, you will need groff or
#    troff, gtbl or tbl, TeX, LaTeX, texi2dvi, and dvips. To format and print
#    the Applications Guide, type `make appguide' (but read `tmac.dif' first,
#    unless you are using groff). To print the Programmer's Guide, type
#    `make guide'.
#  - Finally, if you can't or don't wish to print your own copies, nicely
#    printed and bound copies are available from MIT (see `../../../ORDER.4M').
# Sources for ghostscript, groff, gtbl, TeX, LaTeX, texi2dvi, and dvips are
# all freely available (see `../SOURCES').

# Site-specific variables.

# DOSCHK is a command for checking file and directory names within the current
# directory for MS-DOS compatibility, used by `make html'.  If you have GNU
# doschk, and you wish to perform this check, uncomment the following line:
DOSCHK = find . -print | doschk
# Otherwise, skip the check by uncommenting the next line instead:
# DOSCHK = 

# D2PARGS is a list of options for dvips.  Uncomment one of these to set the
# paper size:
# D2PARGS = -t a4
D2PARGS = -t letter

# T2DARGS is a list of options for texi2dvi.  Uncomment one of these to set the
# page size (the size of the printed area on the paper;  normally this should
# match the paper size specified in D2PARGS):
# T2DARGS = -t @afourpaper
T2DARGS = -t @letterpaper

# HTMLDIR is the directory that contains HTML (hypertext) versions of these
# documents.  Don't worry about this unless you plan to regenerate the HTML
# from the sources in this directory.
HTMLDIR = ../../html
 
# INFODIR is the GNU emacs info directory (needed to `make info').  One of the
# following definitions should be correct.
INFODIR = /usr/info
# INFODIR = /usr/local/info
# INFODIR = /usr/local/emacs/info

# LN is a command that makes the file named by its first argument accessible
# via the name given in its second argument.  If your system supports symbolic
# links, uncomment the next line.
LN = ln -s
# Otherwise uncomment the next line.
# LN = ln

# If you wish to install the info (GNU hypertext documentation) files from this
# package, specify the command needed to format them from the texinfo source
# files.  If you have the GNU `makeinfo' utility (the preferred formatter),
# uncomment the next line.
MAKEINFO = makeinfo
# Otherwise, you can use GNU emacs to do the job by uncommenting the next line.
# MAKEINFO = ./makeinfo.sh

# MANDIR is the root of the man page directory tree.  On most systems, this is
# something like /usr/man or /usr/local/man (type `man man' to find out).
MANDIR = /usr/local/man

# MAN1, MAN3, and MAN5 are the directories in which local man pages for
# section 1 (commands), section 3 (libraries), and section 5 (formats) go.
# You may wish to use $(MANDIR)/manl for all of these; if so, uncomment
# the next three lines.
# MAN1 = $(MANDIR)/manl
# MAN3 = $(MANDIR)/manl
# MAN5 = $(MANDIR)/manl
# Uncomment the next three lines to put the man pages in with the standard
# ones.
MAN1 = $(MANDIR)/man1
MAN3 = $(MANDIR)/man3
MAN5 = $(MANDIR)/man5
# If you want to put the man pages somewhere else, edit `maninst.sh' first.

# PERL is the full pathname of your perl interpreter, needed for `make htmlpg'.
PERL = /usr/bin/perl

# PRINT is the name of the program used to produce listings (including any
# options for the desired formatting).
PRINT = lpr

# PSPRINT is the name of the program that prints PostScript files (needed to
# `make guide' and to `make appguide').
PSPRINT = lpr

# SETDPERMISSIONS is the command needed to make directories created during the
# installation accessible to those who will use them.
SETDPERMISSIONS = chmod 755

# SETPERMISSIONS is the command needed to make the installed files accessible
# to those who will use them.  The value given below makes them readable by
# everyone, and writable by the owner only.  (If you perform the installation
# as `root', `root' is the owner of the installed files.)
SETPERMISSIONS = chmod 644

# TROFF is the name of the program that prints UNIX troff files (needed to
# `make appguide' and for the covers of both guides).  Use `groff' if you have
# GNU groff (the preferred formatter).
TROFF = groff
# Use `ptroff' if you have Adobe TranScript software.
# TROFF = ptroff
# Consult your system administrator if you have neither `groff' nor `ptroff'.
# Other (untested) possibilities are `psroff', `ditroff', `nroff', and `troff'.

# TMAN is the TROFF option needed to load the `man' macro package.  This should
# not need to be changed unless your system is non-standard;  see the file
# `tmac.dif' for comments on a page-numbering bug in some versions of the `man'
# package.
# TMAN = -man
# Use the alternate definition below to get consecutively numbered pages using
# GNU groff.  Omit -rD1 if the final document will be printed on only one side
# of each page.
TMAN = -rC1 -rD1 -man

# TMS is the TROFF option needed to load the `ms' macro package.  Use the
# following definition to get the standard `ms' macros.
# TMS = -ms
# Use the following definition to get the GNU groff version of the `ms' macros.
TMS = -mgs

# It should not be necessary to modify anything below this line.
# -----------------------------------------------------------------------------

.IGNORE:

# `make' or `make install': install the man pages
install:
	./maninst.sh $(MAN1) $(MAN3) $(MAN5) "$(SETPERMISSIONS)"
	$(LN) $(MAN1)/a2m.1 $(MAN1)/ad2m.1
	$(LN) $(MAN1)/a2m.1 $(MAN1)/m2a.1
	$(LN) $(MAN1)/a2m.1 $(MAN1)/md2a.1
	$(LN) $(MAN1)/hrfft.1 $(MAN1)/hrlomb.1
	$(LN) $(MAN1)/hrfft.1 $(MAN1)/hrmem.1
	$(LN) $(MAN1)/hrfft.1 $(MAN1)/hrplot.1
	$(LN) $(MAN1)/plot2d.1 $(MAN1)/plot3d.1
	$(LN) $(MAN1)/setwfdb.1 $(MAN1)/cshsetwfdb.1
	$(LN) $(MAN1)/view.1 $(MAN1)/vsetup.1

# `make appguide': print the man pages (the Applications Guide)
appguide:
	$(TROFF) cover.ag >dbag0.ps
	tex dbag
	dvips -o dbag1.ps dbag.dvi
	tbl appguide.int | $(TROFF) $(TMS) >dbag2.ps
	tbl *.1 *.3 *.5 | $(TROFF) $(TMAN) >dbag3.ps
	latex install
	dvips $(D2PARGS) -o dbag4.ps install.dvi
	latex eval
	dvips $(D2PARGS) -o dbag5.ps eval.dvi
	$(PSPRINT) dbag?.ps

dbag.ps:
	tex dbag
	dvips -o dbag1.ps dbag.dvi
	tbl appguide.int | $(TROFF) $(TMS) >dbag2.ps
	tbl *.1 *.3 *.5 | $(TROFF) $(TMAN) >dbag3.ps
	latex install
	dvips $(D2PARGS) -o dbag4.ps install.dvi
	latex eval
	dvips $(D2PARGS) -o dbag5.ps eval.dvi
	cat dbag[12345].ps >dbag.ps

# `make guide': print the Programmer's Guide
guide:	dbpg.ps
	$(TROFF) cover.pg >dbpg0.ps
	$(PSPRINT) dbpg0.ps dbpg.ps

dbpg.ps:
	texi2dvi $(T2DARGS) dbu.tex
	dvips $(D2PARGS) -o dbpg.ps dbu.dvi

# `make info': create and install emacs info files
info:
	$(MAKEINFO) dbu.tex
	test -s dbpg && \
	 ( test -d $(INFODIR) || \
          ( mkdir -p $(INFODIR); $(SETDPERMISSIONS) $(INFODIR) ); \
         cp dbpg* $(INFODIR); \
         ( test -s $(INFODIR)/dbpg && \
	  ( $(SETPERMISSIONS) $(INFODIR)/dbpg*; \
	   ( test -s $(INFODIR)/dir || cp dir.top $(INFODIR)/dir ); \
	   ( grep -s dbpg $(INFODIR)/dir >/dev/null || \
	     cat dir.db >>$(INFODIR)/dir )))); \
	rm -f dbpg*

# `make html': create HTML files, check for anything not accessible to MSDOS
html:	makehtmldirs htmlag htmlpg
	cd $(HTMLDIR); $(DOSCHK); ln -s index.htm index.html

makehtmldirs:
	-mkdir $(HTMLDIR)
	cp -p index.ht0 $(HTMLDIR)/index.htm
	date '+%e %B %Y' >>$(HTMLDIR)/index.htm
	cat foot.ht0 >>$(HTMLDIR)/index.htm
	-mkdir $(HTMLDIR)/dbpg
	-mkdir $(HTMLDIR)/dbag

htmlag:	makehtmldirs dbag.ps
	cp -p icons/* dbag.ps fixag.sh fixag.sed $(HTMLDIR)/dbag
	./manhtml.sh $(HTMLDIR)/dbag *.1 *.3 *.5
	latex2html -dir $(HTMLDIR)/dbag -local_icons -prefix in install
	latex2html -dir $(HTMLDIR)/dbag -local_icons -prefix ev eval
	rm -f $(HTMLDIR)/dbag/index.html
	cd $(HTMLDIR)/dbag; ./fixag.sh *.html; rm -f fixag.sh images.*
	cd $(HTMLDIR)/dbag; rm -f .I* .ORIG_MAP *.html *.pl fixag.sed
	cp dbag.ht0 $(HTMLDIR)/dbag/dbag.htm
	date '+%e %B %Y' >>$(HTMLDIR)/dbag/dbag.htm
	cat foot.ht0 >>$(HTMLDIR)/dbag/dbag.htm
	cd $(HTMLDIR)/dbag; ln -s dbag.htm index.html
	cp intro.ht0 $(HTMLDIR)/dbag/intro.htm

htmlpg:	makehtmldirs dbpg.ps
	cp -p dbu.tex dbpg.ps $(HTMLDIR)/dbpg
	echo '#!$(PERL)' >$(HTMLDIR)/dbpg/texi2html
	cat texi2html >>$(HTMLDIR)/dbpg/texi2html
	chmod +x $(HTMLDIR)/dbpg/texi2html
	cp dbu.ht0 $(HTMLDIR)/dbpg/dbpg.htm
	cd $(HTMLDIR)/dbpg; ./texi2html -short_ext -menu -split_node dbu.tex
	rm -f $(HTMLDIR)/dbpg/dbu.tex $(HTMLDIR)/dbpg/texi2html
	./fixpg.sh $(HTMLDIR)/dbpg
	date '+%e %B %Y' >>$(HTMLDIR)/dbpg/dbpg.htm
	cat foot.ht0 >>$(HTMLDIR)/dbpg/dbpg.htm
	cd $(HTMLDIR)/dbpg; ln -s dbpg.htm index.html

# `make listing': print listings of programs in this directory
listing:
	$(PRINT) README Makefile fixag.sed fixag.sh fixpg.sed fixpg.sh \
	 makeinfo.sh manhtml.sh maninst.sh tmac.dif
# This directory also contains .latex2html-init, which is a slightly customized
# version of dot.latex2html-init from the latex2html 96.1 distribution; and
# texi2html, included here for convenience since not everyone may have it.
# They are not included in the listings, however, because of their length and
# specialized interest.

# `make clean': remove intermediate and backup files
clean:
	rm -f *.aux *.dvi *.log db*.toc db*.?? db*.??s dbpg* *~ texindex
