# file: Makefile.tpl		G. Moody	  23 May 2000
#				Last revised:	  8 March 2017
# This section of the Makefile should not need to be changed.

DBFILES = 100a.atr 100s.atr 100s.dat *.hea *list wfdbcal

all:
	@echo Nothing to be made in `pwd`.

install: $(DESTDIR)$(DBDIR) $(DESTDIR)$(DBDIR)/pipe $(DESTDIR)$(DBDIR)/tape
	cp $(DBFILES) $(DESTDIR)$(DBDIR)
	cp pipe/* $(DESTDIR)$(DBDIR)/pipe
	cp tape/* $(DESTDIR)$(DBDIR)/tape
	-cd $(DESTDIR)$(DBDIR); $(SETPERMISSIONS) $(DBFILES)
	-cd $(DESTDIR)$(DBDIR); ln -sf wfdbcal dbcal
	-cd $(DESTDIR)$(DBDIR)/pipe; $(SETPERMISSIONS) *
	-cd $(DESTDIR)$(DBDIR)/tape; $(SETPERMISSIONS) *

# 'make collect': retrieve the installed files
collect:
	../conf/collect.sh $(DBDIR) $(DBFILES) wfdbcal dbcal
	cd pipe; ../../conf/collect.sh $(DBDIR)/pipe *
	cd tape; ../../conf/collect.sh $(DBDIR)/tape *
	
uninstall:
	cd pipe; ../../uninstall.sh $(DESTDIR)$(DBDIR)/pipe *
	cd tape; ../../uninstall.sh $(DESTDIR)$(DBDIR)/tape *
	../uninstall.sh $(DESTDIR)$(DBDIR) $(DBFILES) dbcal

$(DESTDIR)$(DBDIR):
	mkdir -p $(DESTDIR)$(DBDIR)
	$(SETDPERMISSIONS) $(DESTDIR)$(DBDIR)
$(DESTDIR)$(DBDIR)/pipe: $(DESTDIR)$(DBDIR)
	mkdir -p $(DESTDIR)$(DBDIR)/pipe
	$(SETDPERMISSIONS) $(DESTDIR)$(DBDIR)/pipe
$(DESTDIR)$(DBDIR)/tape: $(DESTDIR)$(DBDIR)
	mkdir -p $(DESTDIR)$(DBDIR)/tape
	$(SETDPERMISSIONS) $(DESTDIR)$(DBDIR)/tape

listing:
	$(PRINT) README Makefile makefile.dos

clean:
	rm -f *~
