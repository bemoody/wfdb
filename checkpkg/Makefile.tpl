all:
	@rm -f lcheck
	@make lcheck	
	-@./libcheck $(DBDIR) $(LIBDIR) >libcheck.out
	@./appcheck $(INCDIR) $(BINDIR) $(LIBDIR)
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
