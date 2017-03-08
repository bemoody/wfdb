# file: Makefile.tpl		G. Moody	  24 May 2000
#				Last revised:	  8 March 2017
# This section of the Makefile should not need to be changed.

# Programs to be compiled.
XFILES = coherence fft log10 lomb memse

# Shell scripts to be installed.
SCRIPTS = hrfft hrlomb hrmem hrplot plot2d plot3d

# `make all': build applications
all:	$(XFILES)
	$(STRIP) $(XFILES)

# `make' or `make install':  build and install applications
install:	$(DESTDIR)$(BINDIR) all scripts
	$(SETXPERMISSIONS) $(XFILES)
	../install.sh $(DESTDIR)$(BINDIR) $(XFILES)

# 'make collect': retrieve the installed applications
collect:
	../conf/collect.sh $(BINDIR) $(XFILES) $(SCRIPTS)

# `make scripts': customize and install scripts
scripts:	$(DESTDIR)$(BINDIR)
	sed s+BINDIR+$(BINDIR)+g <hrfft >$(DESTDIR)$(BINDIR)/hrfft
	sed s+BINDIR+$(BINDIR)+g <hrlomb >$(DESTDIR)$(BINDIR)/hrlomb
	sed s+BINDIR+$(BINDIR)+g <hrmem >$(DESTDIR)$(BINDIR)/hrmem
	sed s+BINDIR+$(BINDIR)+g <hrplot >$(DESTDIR)$(BINDIR)/hrplot
	cp plot2d plot3d $(DESTDIR)$(BINDIR)
	cd $(DESTDIR)$(BINDIR); $(SETXPERMISSIONS) $(SCRIPTS)

uninstall:
	../uninstall.sh $(DESTDIR)$(BINDIR) $(XFILES) $(SCRIPTS)

coherence:	coherence.c
	$(CC) $(MFLAGS) -o coherence -O coherence.c -lm

fft:		fft.c
	$(CC) $(MFLAGS) -o fft -O fft.c -lm

log10:		log10.c
	$(CC) $(MFLAGS) $(CCDEFS) -o log10 -O log10.c -lm

lomb:		lomb.c
	$(CC) $(MFLAGS) -o lomb -O lomb.c -lm

memse:		memse.c
	$(CC) $(MFLAGS) -o memse -O memse.c -lm

# `make clean': remove intermediate and backup files.
clean:
	rm -f *.o *~ $(XFILES)

# Create directory for installation if necessary.
$(DESTDIR)$(BINDIR):
	mkdir -p $(DESTDIR)$(BINDIR)
	$(SETDPERMISSIONS) $(DESTDIR)$(BINDIR)
