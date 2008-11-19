# file: Makefile.tpl		G. Moody	  24 May 2000
#				Last revised:   19 November 2008
# This section of the Makefile should not need to be changed.

INCLUDES = $(INCDIR)/wfdb/wfdb.h $(INCDIR)/wfdb/ecgcodes.h \
 $(INCDIR)/wfdb/ecgmap.h
HFILES = wfdb.h ecgcodes.h ecgmap.h wfdblib.h
CFILES = wfdbinit.c annot.c signal.c calib.c wfdbio.c
OFILES = wfdbinit.o annot.o signal.o calib.o wfdbio.o
MFILES = Makefile Makefile.dos

# `make' or `make all':  build the WFDB library and wfdb-config
all:	setup $(OFILES)
	rm -f wfdb-config wfdb-config.exe
	$(MAKE) wfdb-config	# be sure that compiled-in paths are up-to-date
	$(BUILDLIB) $(OFILES) $(BUILDLIB_LDFLAGS)

# `make install':  install the WFDB library and headers
install:	$(INCLUDES) $(LIBDIR) $(BINDIR) all
	cp $(WFDBLIB) $(LIBDIR) 
	$(SETLPERMISSIONS) $(LIBDIR)/$(WFDBLIB)
	$(MAKE) lib-post-install
	../install.sh $(BINDIR) wfdb-config

# 'make collect':  retrieve the installed WFDB library and headers
collect:
	../conf/collect.sh $(INCDIR)/wfdb wfdb.h ecgcodes.h ecgmap.h
	../conf/collect.sh $(LIBDIR) $(WFDBLIB) $(WFDBLIB_DLLNAME)
	../conf/collect.sh $(BINDIR) wfdb-config

uninstall:
	../uninstall.sh $(BINDIR) wfdb-config
	../uninstall.sh $(BINDIR)
	../uninstall.sh $(INCDIR)/wfdb $(HFILES)
	../uninstall.sh $(INCDIR)
	../uninstall.sh $(LIBDIR) $(WFDBLIB)
	$(MAKE) lib-post-uninstall
	../uninstall.sh $(LIBDIR)

setup:
	sed "s+DBDIR+$(DBDIR)+" <wfdblib.h0 >wfdblib.h

wfdb-config:
	$(CC) $(CFLAGS) -DVERSION='"$(VERSION)"' -DCFLAGS='"-I$(INCDIR)"' \
	  -DLDFLAGS='"-lwfdb"' -I$(INCDIR) -o $@ wfdb-config.c

# `make clean': remove binaries and backup files
clean:
	rm -f $(OFILES) libwfdb.* *.dll *~ wfdb-config

# `make TAGS':  make an `emacs' TAGS file
TAGS:		$(HFILES) $(CFILES)
	@etags $(HFILES) $(CFILES)

# `make listing':  print a listing of WFDB library sources
listing:
	$(PRINT) README $(MFILES) $(HFILES) $(CFILES) wfdb-config.c

# Rule for creating installation directories
$(BINDIR) $(INCDIR) $(INCDIR)/wfdb $(INCDIR)/ecg $(LIBDIR):
	mkdir -p $@; $(SETDPERMISSIONS) $@

# Rules for installing the include files
$(INCDIR)/wfdb/wfdb.h:		$(INCDIR)/wfdb wfdb.h
	cp -p wfdb.h $(INCDIR)/wfdb; $(SETPERMISSIONS) $(INCDIR)/wfdb/wfdb.h
$(INCDIR)/wfdb/ecgcodes.h:	$(INCDIR)/wfdb ecgcodes.h
	cp -p ecgcodes.h $(INCDIR)/wfdb
	$(SETPERMISSIONS) $(INCDIR)/wfdb/ecgcodes.h
$(INCDIR)/wfdb/ecgmap.h:		$(INCDIR)/wfdb ecgmap.h
	cp -p ecgmap.h $(INCDIR)/wfdb
	$(SETPERMISSIONS) $(INCDIR)/wfdb/ecgmap.h

# Prerequisites for the library modules
wfdbinit.o:	wfdb.h wfdblib.h wfdbinit.c
annot.o:	wfdb.h ecgcodes.h ecgmap.h wfdblib.h annot.c
signal.o:	wfdb.h wfdblib.h signal.c
calib.o:	wfdb.h wfdblib.h calib.c
wfdbio.o:	wfdb.h wfdblib.h wfdbio.c
