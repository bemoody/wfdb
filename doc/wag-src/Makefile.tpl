# file: Makefile.tpl		G. Moody	  24 May 2000
#				Last revised:	 4 March 2003
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

# WAGPSREQ is the target that must be made in order to make the PostScript
# version of the manual (wag.ps), and MAKEWAGPS is the command that must be
# run in order to do this.  The process is a bit convoluted, because the
# simple PostScript version (wag0.ps) is concatenated from several PostScript
# files and thus lacks DSCs (document structuring comments).  wag0.ps can be
# printed or viewed directly, but most (perhaps all) viewers are incapable of
# allowing the user to jump to a random page in a PostScript file that lacks
# DSCs, and it's not easy to select a subset of pages to print in such a
# file.  If you have ghostscript version 7.x or later (earlier versions will
# not work properly), ps2pdf (included with ghostscript) and acroread (or
# pdftops), you can translate wag0.ps into PDF (adding the DSCs in the
# process), and then translate the PDF file back into PostScript with DSCs.
# A disadvantage of this is that the PDF version is roughly 25% larger than
# wag0.ps, and the final PostScript version is nearly twice as large as
# wag0.ps, and takes longer to render as a result.  To enable creation of
# PostScript with DSCs in this way, uncomment the next two lines:
WAGPSREQ = wag.pdf
MAKEWAGPS = acroread -toPostScript -level1 -fast wag.pdf
# You can use pdftops instead of acroread by commenting out the previous line
# and uncommenting the next one.
# MAKEWAGPS = pdftops wag.pdf
# Otherwise, uncomment the next two lines instead:
# WAGPSREQ = wag0.ps
# MAKEWAGPS = cp wag0.ps wag.ps

# It should not be necessary to modify anything below this line.
# -----------------------------------------------------------------------------

.IGNORE:

all:	wag.html wag.ps wag.pdf
	cp -p wag.ps wag.pdf ../wag

install:	wag.man

uninstall:
	../../uninstall.sh $(MAN1) *.1 ad2m.1 ann2rr.1 m2a.1 md2a.1 hrlomb.1 \
	 hrmem.1 hrplot.1 plot3d.1 cshsetwfdb.1 vsetup.1 rr2ann.1 gtkwave.1
	../../uninstall.sh $(MAN3) *.3
	../../uninstall.sh $(MAN5) *.5
	../../uninstall.sh $(MAN7) *.7
	rm -f ../wag/*

# 'make wag-book': print a copy of the WFDB Applications Guide
wag-book:	wag0.ps
	cp wag.cover wagcover
	echo $(SHORTDATE) >>wagcover
	echo .bp >>wagcover
	$(TROFF) wagcover >wagcover.ps
	$(PSPRINT) wagcover.ps
	$(PSPRINT) wag0.ps

# 'make wag.html': format the WFDB Applications Guide as HTML
wag.html:
	cp -p ../misc/icons/* fixag.sh fixag.sed ../wag
	./manhtml.sh ../wag *.1 *.3 *.5 *.7
	cp -p install0.tex install.tex
	cp -p eval0.tex eval.tex
	latex2html -dir ../wag -local_icons -prefix in \
	 -up_url="wag.htm" -up_title="WFDB Applications Guide" install
	latex2html -dir ../wag -local_icons -prefix ev \
	 -up_url="wag.htm" -up_title="WFDB Applications Guide" eval
	rm -f install.tex eval.tex
	cd ../wag; rm -f index.html WARNINGS *.aux *.log *.tex
	sed "s/LONGDATE/$(LONGDATE)/" <intro.ht0 >../wag/intro.htm
	sed "s/LONGDATE/$(LONGDATE)/" <faq.ht0 >../wag/faq.htm
	cd ../wag; ./fixag.sh "$(LONGDATE)" *.html; rm -f fixag.sh images.*
	cd ../wag; rm -f .I* .ORIG_MAP *.html *.pl fixag.sed
	./maketoc-html.sh | \
	  sed "s/LONGDATE/$(LONGDATE)/" | \
	  sed "s/VERSION/$(VERSION)/" >../wag/wag.htm
	cd ../wag; ln -s wag.htm index.html

# 'make wag.man': install the man pages from the WFDB Applications Guide
wag.man:
	test -d $(MAN1) || ( mkdir -p $(MAN1); $(SETDPERMISSIONS) $(MAN1) )
	test -d $(MAN3) || ( mkdir -p $(MAN3); $(SETDPERMISSIONS) $(MAN3) )
	test -d $(MAN5) || ( mkdir -p $(MAN5); $(SETDPERMISSIONS) $(MAN5) )
	test -d $(MAN7) || ( mkdir -p $(MAN7); $(SETDPERMISSIONS) $(MAN7) )
	./maninst.sh $(MAN1) $(MAN3) $(MAN5) $(MAN7) "$(SETPERMISSIONS)"
	cd $(MAN1); $(LN) a2m.1 ad2m.1
	cd $(MAN1); $(LN) a2m.1 ahaconvert.1
	cd $(MAN1); $(LN) a2m.1 m2a.1
	cd $(MAN1); $(LN) a2m.1 md2a.1
	cd $(MAN1); $(LN) ann2rr.1 rr2ann.1
	cd $(MAN1); $(LN) edf2mit.1 mit2edf.1
	cd $(MAN1); $(LN) hrfft.1 hrlomb.1
	cd $(MAN1); $(LN) hrfft.1 hrmem.1
	cd $(MAN1); $(LN) hrfft.1 hrplot.1
	cd $(MAN1); $(LN) plot2d.1 plot3d.1
	cd $(MAN1); $(LN) pnnlist.1 pNNx.1
	cd $(MAN1); $(LN) setwfdb.1 cshsetwfdb.1
	cd $(MAN1); $(LN) view.1 vsetup.1
	cd $(MAN1); $(LN) wav2mit.1 mit2wav.1
	cd $(MAN1); $(LN) wave.1 gtkwave.1

# 'make wag.pdf': format the WFDB Applications Guide as PDF
wag.pdf:	wag0.ps
	ps2pdf wag0.ps wag.pdf

# 'make wag.ps': format the WFDB Applications Guide as PostScript
wag.ps:		$(WAGPSREQ)
	$(MAKEWAGPS)

wag0.ps:	wag.tex
	$(MAKE) wag2.ps
	$(MAKE) wag1.toc
	sed 's/VERSION/$(VERSION)/' <wag.tex | \
	  sed 's/LONGDATE/$(LONGDATE)/' >wag1.tex
	latex wag1					# front matter
	dvips $(D2PARGS) -o wag1.ps wag1
	cat wag[1234].ps | grep -v '^%%' >wag0.ps	# concatenate sections

wag1.toc:	wag2.ps
	$(MAKE) getpagenos maketoclines
	./maketoc-tex.sh >wag1.toc			# TOC and appendices

wag2.ps:
	tbl *.1 *.3 *.5 | $(TROFF) $(TMAN) >wag2.ps	# man pages

install.tex:	wag1.toc
wag3.ps:	install.tex
	sed "s/LONGDATE/$(LONGDATE)/" <install.tex | \
	  sed "s/VERSION/$(VERSION)/" >wag3.tex
	latex wag3
	dvips $(D2PARGS) -o wag3.ps wag3.dvi

eval.tex:	wag1.toc
wag4.ps:	eval.tex
	sed "s/LONGDATE/$(LONGDATE)/" <eval.tex | \
	  sed "s/VERSION/$(VERSION)/" >wag4.tex
	latex wag4
	dvips $(D2PARGS) -o wag4.ps wag4.dvi

# 'make clean': remove intermediate and backup files
clean:
	rm -f *.aux *.dvi *.log *.ps *.toc intro.htm faq.htm wag.pdf wagcover \
	      eval.tex install.tex wag[1234].tex *~
