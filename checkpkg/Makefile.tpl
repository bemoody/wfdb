all: lcheck$(EXEEXT)
	-@./libcheck $(DESTDIR)$(DBDIR) $(DESTDIR)$(LIBDIR) >libcheck.out
	@./appcheck $(DESTDIR)$(INCDIR) $(DESTDIR)$(BINDIR) $(DESTDIR)$(LIBDIR) $(DESTDIR)$(PSPDIR)
	@echo
	@cat libcheck.out appcheck.out
	@grep 'all .* tests passed' libcheck.out > grep.out
	@grep 'all .* tests passed' appcheck.out > grep.out
	@rm -f grep.out

lcheck.exe: lcheck.c
	$(MAKE) lcheck
lcheck:		lcheck.c $(DESTDIR)$(INCDIR)/wfdb/wfdb.h
	@echo Compiling WFDB library test application ...
	@$(CC) $(CFLAGS) lcheck.c -o lcheck$(EXEEXT) $(LDFLAGS) \
	  && echo " Succeeded"

clean:
	rm -f *~ lcheck lcheck.exe libcheck.out appcheck.out
