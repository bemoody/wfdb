# file: Makefile.tpl		G. Moody		24 May 2000
#				Last revised:		5 June 2000
# Change the settings below as appropriate for your setup.

# URLV is the command that starts your web browser if necessary and opens the
# URL named in its first argument.  The following works properly with Netscape
# 1.1 and later versions;  if you are using a different browser, consult its
# documentation.
URLV='( netscape -remote "openURL($$1)" 2>/dev/null || netscape $$1 ) &'

# `make all' creates urlview, wavescript, and wave-remote without installing
# them.
all:	wavescript wave-remote

# `make install' installs `wavescript' and `wave-remote'.  See the WAVE User's
# Guide for instructions on setting up `wavescript' as a helper application for
# your Web browser.
install:	$(BINDIR) urlview wavescript wave-remote
	strip wavescript
	strip wave-remote
	cp urlview wavescript wave-remote $(BINDIR)
	$(SETXPERMISSIONS) $(BINDIR)/urlview $(BINDIR)/wavescript \
	 $(BINDIR)/wave-remote

uninstall:
	../uninstall $(BINDIR) urlview wavescript wave-remote

# `urlview' opens a web browser to view a named URL.
urlview:	Makefile
	cp urlvhead urlview
	echo $(URLV) >>urlview

# `wavescript' reads commands from a named file and passes them to WAVE.
wavescript:	wavescript.c
	$(CC) -o wavescript -DBINDIR=$(BINDIR) -O wavescript.c

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

# Create directory for installation if necessary.
$(BINDIR):
	mkdir -p $(BINDIR); $(SETDPERMISSIONS) $(BINDIR)
