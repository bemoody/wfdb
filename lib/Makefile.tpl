# file: Makefile.tpl		G. Moody	  24 May 2000
#				Last revised:   28 November 2004
# This section of the Makefile should not need to be changed.

INCLUDES = $(INCDIR)/wfdb/wfdb.h $(INCDIR)/wfdb/ecgcodes.h \
 $(INCDIR)/wfdb/ecgmap.h
COMPAT_INCLUDES = $(INCDIR)/ecg/db.h $(INCDIR)/ecg/ecgcodes.h \
 $(INCDIR)/ecg/ecgmap.h
HFILES = wfdb.h ecgcodes.h ecgmap.h wfdblib.h
CFILES = wfdbinit.c annot.c signal.c calib.c wfdbio.c
OFILES = wfdbinit.o annot.o signal.o calib.o wfdbio.o
MFILES = Makefile Makefile.dos

# `make' or `make all':  build the WFDB library and wfdb-config
all:	$(OFILES) wfdb-config
	$(BUILDLIB) $(OFILES) $(BUILDLIB_LDFLAGS)

# `make install':  install the WFDB library and headers
install:	$(INCLUDES) $(LIBDIR) $(BINDIR) all
	cp $(WFDBLIB) $(LIBDIR) 
	$(SETLPERMISSIONS) $(LIBDIR)/$(WFDBLIB)
	$(MAKE) lib-post-install
	../install.sh $(BINDIR) wfdb-config

uninstall:
	../uninstall.sh $(BINDIR) wfdb-config
	../uninstall.sh $(BINDIR)
	../uninstall.sh $(INCDIR)/wfdb $(HFILES)
	../uninstall.sh $(INCDIR)
	../uninstall.sh $(LIBDIR) $(WFDBLIB)
	$(MAKE) lib-post-uninstall
	../uninstall.sh $(LIBDIR)

wfdb-config:	wfdb-config.c Makefile
	$(CC) -DVERSION='"$(VERSION)"' -DCFLAGS='"-I$(INCDIR)"' \
	  -DLDFLAGS='"$(LDFLAGS)"' -I$(INCDIR) -o $@ wfdb-config.c

# `make compat':  install the includes needed for source compatibility with
# applications written for pre-version 10.0.0 versions of this library
compat:		$(INCLUDES) $(COMPAT_INCLUDES)

# `make clean':  also remove previously compiled versions of the library
clean:
	rm -f $(OFILES) libwfdb.* *.dll *~ wfdb.h wfdblib.h wfdb-config

# `make TAGS':  make an `emacs' TAGS file
TAGS:		$(HFILES) $(CFILES)
	@etags $(HFILES) $(CFILES)

# `make listing':  print a listing of WFDB library sources
listing:
	$(PRINT) README $(MFILES) $(HFILES) $(CFILES) wfdb-config.c

# Rules for creating installation directories
$(BINDIR):
	mkdir -p $(BINDIR); $(SETDPERMISSIONS) $(BINDIR)
$(INCDIR):
	mkdir -p $(INCDIR); $(SETDPERMISSIONS) $(INCDIR)
$(INCDIR)/wfdb:	$(INCDIR)
	mkdir -p $(INCDIR)/wfdb; $(SETDPERMISSIONS) $(INCDIR)/wfdb
$(INCDIR)/ecg:	$(INCDIR)
	mkdir -p $(INCDIR)/ecg; $(SETDPERMISSIONS) $(INCDIR)/ecg
$(LIBDIR):
	mkdir -p $(LIBDIR); $(SETDPERMISSIONS) $(LIBDIR)

# Rules for installing the include files
$(INCDIR)/wfdb/wfdb.h:		$(INCDIR)/wfdb wfdb.h
	cp -p wfdb.h $(INCDIR)/wfdb; $(SETPERMISSIONS) $(INCDIR)/wfdb/wfdb.h
$(INCDIR)/wfdb/ecgcodes.h:	$(INCDIR)/wfdb ecgcodes.h
	cp -p ecgcodes.h $(INCDIR)/wfdb
	$(SETPERMISSIONS) $(INCDIR)/wfdb/ecgcodes.h
$(INCDIR)/wfdb/ecgmap.h:		$(INCDIR)/wfdb ecgmap.h
	cp -p ecgmap.h $(INCDIR)/wfdb
	$(SETPERMISSIONS) $(INCDIR)/wfdb/ecgmap.h

# Rules for installing the compatibility (pre-10.0.0) include files
$(INCDIR)/ecg/db.h:		$(INCDIR)/ecg db.h
	cp -p db.h $(INCDIR)/ecg; $(SETPERMISSIONS) $(INCDIR)/ecg/db.h
$(INCDIR)/ecg/ecgcodes.h:	$(INCDIR)/ecg $(INCDIR)/wfdb/ecgcodes.h
	ln -s $(INCDIR)/wfdb/ecgcodes.h $(INCDIR)/ecg/ecgcodes.h
$(INCDIR)/ecg/ecgmap.h:		$(INCDIR)/ecg $(INCDIR)/wfdb/ecgmap.h
	ln -s $(INCDIR)/wfdb/ecgmap.h $(INCDIR)/ecg/ecgmap.h

# Prerequisites for the library modules
wfdbinit.o:	wfdb.h wfdblib.h wfdbinit.c
annot.o:	wfdb.h ecgcodes.h ecgmap.h wfdblib.h annot.c
signal.o:	wfdb.h wfdblib.h signal.c
calib.o:	wfdb.h wfdblib.h calib.c
wfdbio.o:	wfdb.h wfdblib.h wfdbio.c
