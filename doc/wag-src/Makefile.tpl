# file: Makefile.tpl		G. Moody	 24 May 2000
#				Last revised:  20 December 2001
# Change the settings below as appropriate for your setup.

# D2PARGS is a list of options for dvips.  Uncomment one of these to set the
# paper size ("a4" is most common except in the US and Canada):
# D2PARGS = -t a4
D2PARGS = -t letter

# LN is a command that makes the file named by its first argument accessible
# via the name given in its second argument.  If your system supports symbolic
# links, uncomment the next line.
LN = ln -sf
# Otherwise uncomment the next line if your system supports hard links.
# LN = ln
# If your system doesn't support links at all, copy files instead.
# LN = cp

# MANDIR is the root of the man page directory tree.  On most systems, this is
# something like /usr/man or /usr/local/man (type 'man man' to find out).
MANDIR = /usr/local/man

# MAN1, MAN3, MAN5, and MAN7 are the directories in which local man pages for
# section 1 (commands), section 3 (libraries), section 5 (formats), and
# section 7 (conventions and miscellany) go.  You may wish to use
# $(MANDIR)/manl for all of these; if so, uncomment the next four lines.
# MAN1 = $(MANDIR)/manl
# MAN3 = $(MANDIR)/manl
# MAN5 = $(MANDIR)/manl
# MAN7 = $(MANDIR)/manl
# Uncomment the next four lines to put the man pages in with the standard
# ones.
MAN1 = $(MANDIR)/man1
MAN3 = $(MANDIR)/man3
MAN5 = $(MANDIR)/man5
MAN7 = $(MANDIR)/man7
# If you want to put the man pages somewhere else, edit 'maninst.sh' first.

# PSPRINT is the name of the program that prints PostScript files. If your
# printer is not a PostScript printer, see the GhostScript documentation to see
# how to do this (since the figure files are in PostScript form, it is not
# sufficient to use a non-PostScript dvi translator such as dvilj).
PSPRINT = lpr

# TROFF is the name of the program that prints UNIX troff files (needed to
# 'make ag' and for the covers of the guides).  Use 'groff' if you have
# GNU groff (the preferred formatter).
TROFF = groff
# Use 'ptroff' if you have Adobe TranScript software.
# TROFF = ptroff
# Consult your system administrator if you have neither 'groff' nor 'ptroff'.
# Other (untested) possibilities are 'psroff', 'ditroff', 'nroff', and 'troff'.

# TMAN is the TROFF option needed to load the 'man' macro package.  This should
# not need to be changed unless your system is non-standard;  see the file
# 'tmac.dif' for comments on a page-numbering bug in some versions of the 'man'
# package.
# TMAN = -man
# Use the alternate definition below to get consecutively numbered pages using
# GNU groff.  Omit -rD1 if the final document will be printed on only one side
# of each page.
TMAN = -rC1 -rD1 -man

# TMS is the TROFF option needed to load the 'ms' macro package.  Use the
# following definition to get the standard 'ms' macros.
# TMS = -ms
# Use the following definition to get the GNU groff version of the 'ms' macros.
TMS = -mgs

# It should not be necessary to modify anything below this line.
# -----------------------------------------------------------------------------

.IGNORE:

all:	wag.html wag.ps wag.pdf
	cp -p wag.ps wag.pdf ../wag

install:	wag.man

uninstall:
	../../uninstall.sh $(MAN1) *.1 ad2m.1 ann2rr.1 m2a.1 md2a.1 hrlomb.1 \
	 hrmem.1 hrplot.1 plot3d.1 cshsetwfdb.1 vsetup.1 rr2ann.1
	../../uninstall.sh $(MAN3) *.3
	../../uninstall.sh $(MAN5) *.5
	../../uninstall.sh $(MAN7) *.7
	rm -f ../wag/*

# 'make wag-book': print a copy of the WFDB Applications Guide
wag-book:	wag.ps
	$(TROFF) wag.cover >wagcover.ps
	$(PSPRINT) wagcover.ps wag.ps

# 'make wag.html': format the WFDB Applications Guide as HTML
wag.html:
	cp -p ../misc/icons/* fixag.sh fixag.sed ../wag
	./manhtml.sh ../wag *.1 *.3 *.5 *.7
	latex2html -dir ../wag -local_icons -prefix in \
	 -up_url="wag.htm" -up_title="WFDB Applications Guide" install
	latex2html -dir ../wag -local_icons -prefix ev \
	 -up_url="wag.htm" -up_title="WFDB Applications Guide" eval
	rm -f ../wag/index.html
	cd ../wag; ./fixag.sh *.html; rm -f fixag.sh images.*
	cd ../wag; rm -f .I* .ORIG_MAP *.html *.pl fixag.sed
	cp wag.ht0 ../wag/wag.htm
	date '+%e %B %Y' >>../wag/wag.htm
	cat ../misc/foot.ht0 >>../wag/wag.htm
	cd ../wag; ln -s wag.htm index.html
	cp intro.ht0 ../wag/intro.htm

# 'make wag.man': install the man pages from the WFDB Applications Guide
wag.man:
	test -d $(MAN1) || ( mkdir -p $(MAN1); $(SETDPERMISSIONS) $(MAN1) )
	test -d $(MAN3) || ( mkdir -p $(MAN3); $(SETDPERMISSIONS) $(MAN3) )
	test -d $(MAN5) || ( mkdir -p $(MAN5); $(SETDPERMISSIONS) $(MAN5) )
	test -d $(MAN7) || ( mkdir -p $(MAN7); $(SETDPERMISSIONS) $(MAN7) )
	./maninst.sh $(MAN1) $(MAN3) $(MAN5) $(MAN7) "$(SETPERMISSIONS)"
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

# 'make wag.pdf': format the WFDB Applications Guide as PDF
wag.pdf:	wag.ps
	ps2pdf wag.ps wag.pdf
	# The PDF produced this way is not so great -- a better way
	# of generating PDF will be implemented in the future

# 'make ag.ps': format the WFDB Applications Guide as PostScript
wag.ps:
	latex wag
	dvips -o wag1.ps wag.dvi
	tbl appguide.int | $(TROFF) $(TMS) >wag2.ps
	tbl *.1 *.3 *.5 | $(TROFF) $(TMAN) >wag3.ps
	latex install
	dvips $(D2PARGS) -o wag4.ps install.dvi
	latex eval
	dvips $(D2PARGS) -o wag5.ps eval.dvi
	cat wag[12345].ps | grep -v '^%%' >wag.ps

# 'make clean': remove intermediate and backup files
clean:
	rm -f *.aux *.dvi *.log *.ps *.toc wag.pdf *~
