# file: Makefile.tpl		G. Moody		23 May 2000
#				Last revised:		5 June 2000
# This section of the Makefile should not need to be changed.

DBFILES = 100s.dat 100s.atr *.hea *list wfdbcal

all:
	@echo Nothing to be made in `pwd`.

install:	$(DBDIR)
	cp $(DBFILES) $(DBDIR)
	-cd $(DBDIR); $(SETPERMISSIONS) $(DBFILES)
	-cd $(DBDIR); ln -sf wfdbcal dbcal

uninstall:
	../uninstall.sh $(DBDIR) $(DBFILES) dbcal

$(DBDIR):
	mkdir $(DBDIR); $(SETDPERMISSIONS) $(DBDIR)

listing:
	$(PRINT) README Makefile makefile.dos

clean:
	rm -f *~
