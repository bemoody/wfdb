# file: Makefile.tpl		G. Moody	23 May 2000
#				Last revised: 17 February 2003
# This section of the Makefile should not need to be changed.

CFILES = ann2rr.c bxb.c calsig.c ecgeval.c epicmp.c fir.c ihr.c mfilt.c \
 mrgann.c mxm.c nguess.c nst.c plotstm.c pscgen.c pschart.c psfd.c rdann.c \
 rdsamp.c rr2ann.c rxr.c sampfreq.c sample.c sigamp.c sigavg.c skewedit.c \
 snip.c sortann.c sqrs.c sqrs125.c sumann.c sumstats.c tach.c view.c vsetup.c \
 wabp.c wfdbcat.c wfdbcollate.c wfdb-config.c wfdbdesc.c wfdbwhich.c wqrs.c \
 wrann.c wrsamp.c wvscript.c xform.c
XFILES = ann2rr bxb calsig ecgeval epicmp fir ihr mfilt \
 mrgann mxm nguess nst plotstm pscgen pschart psfd rdann \
 rdsamp rr2ann rxr sampfreq sigamp sigavg skewedit \
 snip sortann sqrs sqrs125 sumann sumstats tach \
 wabp wfdbcat wfdbcollate wfdb-config wfdbdesc wfdbwhich wqrs \
 wrann wrsamp xform
SCRIPTS = cshsetwfdb setwfdb
PSFILES = pschart.pro psfd.pro 12lead.pro
OTHERFILES = cshsetwfdb setwfdb setwfdb.bat sample8.hea
MFILES = Makefile Makefile.dos

# General rule for compiling C sources into executable files.  This is
# redundant for most versions of `make', but at least one System V version
# needs it.
.c:
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# `make all': build applications
all:	$(XFILES)
	$(STRIP) $(XFILES)

# `make' or `make install':  build and install applications, clean up
install:	all $(BINDIR) $(PSPDIR) scripts
	$(SETXPERMISSIONS) $(XFILES)
	../install.sh $(BINDIR) $(XFILES)
	cp $(PSFILES) $(PSPDIR)
	cd $(PSPDIR); $(SETPERMISSIONS) $(PSFILES)
	$(MAKE) clean

# `make scripts': install customized scripts for setting WFDB path
scripts:
	sed s+/usr/local/database+$(DBDIR)+g <setwfdb >$(BINDIR)/setwfdb
	sed s+/usr/local/database+$(DBDIR)+g <cshsetwfdb >$(BINDIR)/cshsetwfdb
	$(SETPERMISSIONS) $(SCRIPTS)

uninstall:
	../uninstall.sh $(PSPDIR) $(PSFILES)
	../uninstall.sh $(BINDIR) $(XFILES) $(SCRIPTS)
	../uninstall.sh $(LIBDIR)

# Create directories for installation if necessary.
$(BINDIR):
	mkdir -p $(BINDIR); $(SETDPERMISSIONS) $(BINDIR)
$(PSPDIR):
	mkdir -p $(PSPDIR); $(SETDPERMISSIONS) $(PSPDIR)

# `make clean':  remove intermediate and backup files
clean:
	rm -f $(XFILES) *.o *~

# `make listing':  print a listing of WFDB applications sources
listing:
	$(PRINT) README $(MFILES) $(CFILES) $(PSFILES) $(OTHERFILES)

# Rules for compiling applications that require non-standard options

bxb:		bxb.c
	$(CC) $(CFLAGS) bxb.c -o $@ $(LDFLAGS) -lm
mxm:		mxm.c
	$(CC) $(CFLAGS) mxm.c -o $@ $(LDFLAGS) -lm
nguess:		nguess.c
	$(CC) $(CFLAGS) nguess.c -o $@ $(LDFLAGS) -lm
nst:		nst.c
	$(CC) $(CFLAGS) nst.c -o $@ $(LDFLAGS) -lm
plotstm:	plotstm.c
	$(CC) $(CFLAGS) plotstm.c -o $@
pschart:	pschart.c
	$(CC) $(CFLAGS) -DPROLOG=\"$(PSPDIR)/pschart.pro\" pschart.c -o $@ \
          $(LDFLAGS)
psfd:		psfd.c
	$(CC) $(CFLAGS) -DPROLOG=\"$(PSPDIR)/psfd.pro\" psfd.c -o $@ $(LDFLAGS)
sigamp:		sigamp.c
	$(CC) $(CFLAGS) sigamp.c -o $@ $(LDFLAGS) -lm
wfdb-config:	wfdb-config.c Makefile
	$(CC) -DVERSION='"$(VERSION)"' -DCFLAGS='"-I$(INCDIR)"' \
	  -DLDFLAGS='"$(LDFLAGS)"' -o $@ wfdb-config.c
wqrs:		wqrs.c
	$(CC) $(CFLAGS) wqrs.c -o $@ $(LDFLAGS) -lm
