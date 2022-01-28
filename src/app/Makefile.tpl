# file: Makefile.tpl		G. Moody	  23 May 2000
#				Last revised:	 24 April 2020
# This section of the Makefile should not need to be changed.

CFILES = ann2rr.c bxb.c calsig.c ecgeval.c epicmp.c fir.c gqfuse.c gqpost.c \
 gqrs.c hrstats.c ihr.c mfilt.c mrgann.c mxm.c nguess.c nst.c plotstm.c \
 pscgen.c pschart.c psfd.c rdann.c rdsamp.c rr2ann.c rxr.c sampfreq.c sigamp.c \
 sigavg.c signame.c signum.c skewedit.c snip.c sortann.c sqrs.c sqrs125.c \
 stepdet.c sumann.c sumstats.c tach.c time2sec.c wabp.c wfdb-config.c \
 wfdbcat.c wfdbcollate.c wfdbdesc.c wfdbmap.c wfdbsignals.c wfdbtime.c \
 wfdbwhich.c wqrs.c wrann.c wrsamp.c xform.c
CFFILES = gqrs.conf
HFILES = signal-colors.h
XFILES = \
 ann2rr$(EXEEXT) \
 bxb$(EXEEXT) \
 calsig$(EXEEXT) \
 ecgeval$(EXEEXT) \
 epicmp$(EXEEXT) \
 fir$(EXEEXT) \
 gqfuse$(EXEEXT) \
 gqpost$(EXEEXT) \
 gqrs$(EXEEXT) \
 hrstats$(EXEEXT) \
 ihr$(EXEEXT) \
 mfilt$(EXEEXT) \
 mrgann$(EXEEXT) \
 mxm$(EXEEXT) \
 nguess$(EXEEXT) \
 nst$(EXEEXT) \
 plotstm$(EXEEXT) \
 pscgen$(EXEEXT) \
 pschart$(EXEEXT) \
 psfd$(EXEEXT) \
 rdann$(EXEEXT) \
 rdsamp$(EXEEXT) \
 rr2ann$(EXEEXT) \
 rxr$(EXEEXT) \
 sampfreq$(EXEEXT) \
 sigamp$(EXEEXT) \
 sigavg$(EXEEXT) \
 signame$(EXEEXT) \
 signum$(EXEEXT) \
 skewedit$(EXEEXT) \
 snip$(EXEEXT) \
 sortann$(EXEEXT) \
 sqrs$(EXEEXT) \
 sqrs125$(EXEEXT) \
 stepdet$(EXEEXT) \
 sumann$(EXEEXT) \
 sumstats$(EXEEXT) \
 tach$(EXEEXT) \
 time2sec$(EXEEXT) \
 wabp$(EXEEXT) \
 wfdb-config$(EXEEXT) \
 wfdbcat$(EXEEXT) \
 wfdbcollate$(EXEEXT) \
 wfdbdesc$(EXEEXT) \
 wfdbmap$(EXEEXT) \
 wfdbsignals$(EXEEXT) \
 wfdbtime$(EXEEXT) \
 wfdbwhich$(EXEEXT) \
 wqrs$(EXEEXT) \
 wrann$(EXEEXT) \
 wrsamp$(EXEEXT) \
 xform$(EXEEXT)
SCRIPTS = cshsetwfdb setwfdb pnwlogin
PSFILES = pschart.pro psfd.pro 12lead.pro
MFILES = Makefile

# General rule for compiling C sources into executable files.  This is
# redundant for most versions of `make', but at least one System V version
# needs it.
.c:
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# `make all': build applications
all:	$(XFILES)
	$(STRIP) $(XFILES)

# `make' or `make install':  build and install applications
install:	all $(DESTDIR)$(BINDIR) $(DESTDIR)$(PSPDIR) scripts
	rm -f pschart psfd pschart.exe psfd.exe
# be sure compiled-in paths are up-to-date
	$(MAKE) pschart$(EXEEXT) psfd$(EXEEXT)
	$(STRIP) pschart$(EXEEXT) psfd$(EXEEXT)
	$(SETXPERMISSIONS) $(XFILES)
	../install.sh $(DESTDIR)$(BINDIR) $(XFILES)
	cp $(PSFILES) $(DESTDIR)$(PSPDIR)
	cd $(DESTDIR)$(PSPDIR); $(SETPERMISSIONS) $(PSFILES)

# 'make collect': retrieve the installed applications
collect:
	../conf/collect.sh $(BINDIR) $(XFILES) $(SCRIPTS)
	../conf/collect.sh $(PSPDIR) $(PSFILES)

# `make scripts': install customized scripts for setting WFDB path
scripts: $(DESTDIR)$(BINDIR)
	sed s+/usr/local/database+$(DBDIR)+g <setwfdb >$(DESTDIR)$(BINDIR)/setwfdb
	sed s+/usr/local/database+$(DBDIR)+g <cshsetwfdb >$(DESTDIR)$(BINDIR)/cshsetwfdb
	sed s+/usr/local/database+$(DBDIR)+g <pnwlogin >$(DESTDIR)$(BINDIR)/pnwlogin
	cd $(DESTDIR)$(BINDIR); $(SETPERMISSIONS) *setwfdb; $(SETXPERMISSIONS) pnwlogin

uninstall:
	../uninstall.sh $(DESTDIR)$(PSPDIR) $(PSFILES)
	../uninstall.sh $(DESTDIR)$(BINDIR) $(XFILES) $(SCRIPTS)
	../uninstall.sh $(DESTDIR)$(LIBDIR)

# Create directories for installation if necessary.
$(DESTDIR)$(BINDIR):
	mkdir -p $(DESTDIR)$(BINDIR)
	$(SETDPERMISSIONS) $(DESTDIR)$(BINDIR)
$(DESTDIR)$(PSPDIR):
	mkdir -p $(DESTDIR)$(PSPDIR)
	$(SETDPERMISSIONS) $(DESTDIR)$(PSPDIR)

# `make clean':  remove intermediate and backup files
clean:
	rm -f $(XFILES) *.o *~

# `make listing':  print a listing of WFDB applications sources
listing:
	$(PRINT) README $(MFILES) $(CFILES) $(HFILES) $(CFFILES) $(PSFILES)

# Rules for compiling applications that require non-standard options

bxb$(EXEEXT):		bxb.c
	$(CC) $(CFLAGS) bxb.c -o $@ $(LDFLAGS) -lm
ihr$(EXEEXT):		ihr.c
	$(CC) $(CFLAGS) ihr.c -o $@ $(LDFLAGS) -lm
hrstats$(EXEEXT):	hrstats.c
	$(CC) $(CFLAGS) hrstats.c -o $@ $(LDFLAGS) -lm
mxm$(EXEEXT):		mxm.c
	$(CC) $(CFLAGS) mxm.c -o $@ $(LDFLAGS) -lm
nguess$(EXEEXT):	nguess.c
	$(CC) $(CFLAGS) nguess.c -o $@ $(LDFLAGS) -lm
nst$(EXEEXT):		nst.c
	$(CC) $(CFLAGS) nst.c -o $@ $(LDFLAGS) -lm
plotstm$(EXEEXT):	plotstm.c
	$(CC) $(CFLAGS) plotstm.c -o $@
pschart$(EXEEXT):
	$(CC) $(CFLAGS) -DPROLOG=\"$(PSPDIR)/pschart.pro\" pschart.c -o $@ \
          $(LDFLAGS)
psfd$(EXEEXT):
	$(CC) $(CFLAGS) -DPROLOG=\"$(PSPDIR)/psfd.pro\" psfd.c -o $@ $(LDFLAGS)
sigamp$(EXEEXT):	sigamp.c
	$(CC) $(CFLAGS) sigamp.c -o $@ $(LDFLAGS) -lm
wfdbmap$(EXEEXT):	wfdbmap.c signal-colors.h
	$(CC) $(CFLAGS) wfdbmap.c -o $@ $(LDFLAGS)
wqrs$(EXEEXT):		wqrs.c
	$(CC) $(CFLAGS) wqrs.c -o $@ $(LDFLAGS) -lm
