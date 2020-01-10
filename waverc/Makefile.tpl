# file: Makefile.tpl		G. Moody		24 May 2000
#				Last revised:		24 April 2020
# Change the settings below as appropriate for your setup.

XFILES = wavescript$(EXEEXT) wave-remote$(EXEEXT)

# `make all' creates wavescript and wave-remote without installing them.
all:	$(XFILES)

# `make install' installs `url_view', `wavescript', and `wave-remote'.  See
# the WAVE User's Guide for instructions on setting up `wavescript' as a helper
# application for  your Web browser.
install:	$(DESTDIR)$(BINDIR)
	rm -f wavescript wavescript.exe
	$(MAKE) $(XFILES)	# make sure wavescript has the correct BINDIR
	$(STRIP) $(XFILES)
	$(SETXPERMISSIONS) url_view $(XFILES)
	../install.sh $(DESTDIR)$(BINDIR) url_view $(XFILES)

# 'make collect': retrieve the installed applications
collect:
	../conf/collect.sh $(BINDIR) url_view $(XFILES)

uninstall:
	../uninstall.sh $(DESTDIR)$(BINDIR) url_view $(XFILES)

# `wavescript' reads commands from a named file and passes them to WAVE.
wavescript$(EXEEXT):	wavescript.c
	$(CC) $(CFLAGS) -o wavescript$(EXEEXT) \
	  -DBINDIR=$(BINDIR) -O wavescript.c

# `wave-remote' passes its command-line arguments as commands to WAVE.
wave-remote$(EXEEXT):	wave-remote.c
	$(CC) $(CFLAGS) -o wave-remote$(EXEEXT) -O wave-remote.c

# `wave-remote-test' looks like WAVE to `wavescript' and `wave-remote', and
# can be used to verify their proper operation.  Start `wave-remote-test'
# before starting `wavescript' or `wave-remote';  the commands these programs
# send to WAVE should appear on the standard output of `wave-remote-test.
wave-remote-test$(EXEEXT):	wave-remote-test.c
	$(CC) $(CFLAGS) -o wave-remote-test$(EXEEXT) -O wave-remote-test.c

# `make clean':  remove intermediate and backup files
clean:
	rm -f $(XFILES) wave-remote-test$(EXEEXT) *~

# Create directory for installation if necessary.
$(DESTDIR)$(BINDIR):
	mkdir -p $(DESTDIR)$(BINDIR)
	$(SETDPERMISSIONS) $(DESTDIR)$(BINDIR)
