all:
	@rm -f lcheck
	@make lcheck	
	-@./libcheck $(DESTDIR)$(DBDIR) $(DESTDIR)$(LIBDIR) >libcheck.out
	@./appcheck $(DESTDIR)$(INCDIR) $(DESTDIR)$(BINDIR) $(DESTDIR)$(LIBDIR) $(DESTDIR)$(PSPDIR)
	@echo
	@cat libcheck.out appcheck.out
	@grep 'all .* tests passed' libcheck.out > grep.out
	@grep 'all .* tests passed' appcheck.out > grep.out
	@rm -f grep.out

lcheck:	lcheck.c
	@echo Compiling WFDB library test application ...
	@$(CC) $(CFLAGS) lcheck.c -o $@ $(LDFLAGS) && echo " Succeeded"

clean:
	rm -f *~ lcheck libcheck.out appcheck.out
