# file: Makefile.tpl		G. Moody	 24 May 2000
#				Last revised:  12 December 2000
# Change the settings below as appropriate for your setup.

# DOSCHK is a command for checking file and directory names within the current
# directory for MS-DOS compatibility, used by `make html'.  If you have GNU
# doschk, and you wish to perform this check, uncomment the following line:
DOSCHK = find . -print | doschk
# Otherwise, skip the check by uncommenting the next line instead:
# DOSCHK = 

# D2PARGS is a list of options for dvips.  Uncomment one of these to set the
# paper size ("a4" is most common except in the US and Canada):
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
# Otherwise uncomment the next line if your system supports hard links.
# LN = ln
# If your system doesn't support links at all, copy files instead.
# LN = cp

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

# PSPRINT is the name of the program that prints PostScript files (needed to
# `make guide' and to `make appguide').
PSPRINT = lpr

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

all:

# `make' or `make install': install the man pages
install:	$(MAN1) $(MAN3) $(MAN5)
	./maninst.sh $(MAN1) $(MAN3) $(MAN5) "$(SETPERMISSIONS)"
	$(LN) $(MAN1)/a2m.1 $(MAN1)/ad2m.1
	$(LN) $(MAN1)/a2m.1 $(MAN1)/m2a.1
	$(LN) $(MAN1)/a2m.1 $(MAN1)/md2a.1
	$(LN) $(MAN1)/ann2rr.1 $(MAN1)/rr2ann.1
	$(LN) $(MAN1)/hrfft.1 $(MAN1)/hrlomb.1
	$(LN) $(MAN1)/hrfft.1 $(MAN1)/hrmem.1
	$(LN) $(MAN1)/hrfft.1 $(MAN1)/hrplot.1
	$(LN) $(MAN1)/plot2d.1 $(MAN1)/plot3d.1
	$(LN) $(MAN1)/setwfdb.1 $(MAN1)/cshsetwfdb.1
	$(LN) $(MAN1)/view.1 $(MAN1)/vsetup.1

uninstall:
	../uninstall.sh $(MAN1) *.1 ad2m.1 ann2rr.1 m2a.1 md2a.1 hrlomb.1 \
	 hrmem.1 hrplot.1 plot3d.1 cshsetwfdb.1 vsetup.1
	../uninstall.sh $(MAN3) *.3
	../uninstall.sh $(MAN5) *.5

# Create directories for installation if necessary.
$(MAN1):
	mkdir -p $(MAN1); $(SETDPERMISSIONS) $(MAN1)
$(MAN3):
	mkdir -p $(MAN3); $(SETDPERMISSIONS) $(MAN3)
$(MAN5):
	mkdir -p $(MAN5); $(SETDPERMISSIONS) $(MAN5)

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
	cat dbag[12345].ps | grep -v '^%%' >dbag.ps

# `make guide': print the Programmer's Guide
guide:	dbpg.ps
	$(TROFF) cover.pg >dbpg0.ps
	$(PSPRINT) dbpg0.ps dbpg.ps

dbpg.ps:
	texi2dvi $(T2DARGS) dbu.tex
	dvips $(D2PARGS) -o dbpg.ps dbu.dvi

# `make info': create and install emacs info files
info:	$(INFODIR)
	$(MAKEINFO) dbu.tex
	test -s dbpg && \
         cp dbpg* $(INFODIR); \
         ( test -s $(INFODIR)/dbpg && \
	  ( $(SETPERMISSIONS) $(INFODIR)/dbpg*; \
	   ( test -s $(INFODIR)/dir || cp dir.top $(INFODIR)/dir ); \
	   ( grep -s dbpg $(INFODIR)/dir >/dev/null || \
	     cat dir.db >>$(INFODIR)/dir ))); \
	rm -f dbpg*

$(INFODIR):
	mkdir -p $(INFODIR); $(SETDPERMISSIONS) $(INFODIR)

# `make html': create HTML files, check for anything not accessible to MSDOS
html:	makehtmldirs htmlag htmlpg
	cd $(HTMLDIR); $(DOSCHK); ln -s index.htm index.html

makehtmldirs:  $(HTMLDIR)/dbpg $(HTMLDIR)/dbag
	-mkdir $(HTMLDIR)
	cp -p index.ht0 $(HTMLDIR)/index.htm
	date '+%e %B %Y' >>$(HTMLDIR)/index.htm
	cat foot.ht0 >>$(HTMLDIR)/index.htm
	-mkdir $(HTMLDIR)/dbpg
	-mkdir $(HTMLDIR)/dbag

$(HTMLDIR):
	mkdir -p $(HTMLDIR); $(SETDPERMISSIONS) $(HTMLDIR)
$(HTMLDIR)/dbag:	$(HTMLDIR)
	mkdir -p $(HTMLDIR)/dbag; $(SETDPERMISSIONS) $(HTMLDIR)/dbag
$(HTMLDIR)/dbpg:	$(HTMLDIR)
	mkdir -p $(HTMLDIR)/dbpg; $(SETDPERMISSIONS) $(HTMLDIR)/dbpg

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
