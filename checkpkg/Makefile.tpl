all:
	@rm -f lcheck
	@make lcheck	
	-@./libcheck $(DBDIR) $(LIBDIR)
	@../conf/prompt "Press <Enter> to test applications:"; read A
	@./appcheck $(INCDIR) $(BINDIR) $(LIBDIR)

lcheck:	lcheck.c
	@echo Compiling WFDB library test application ...
	@$(CC) $(CFLAGS) lcheck.c -o $@ $(LDFLAGS) && echo " Succeeded"

clean:
	rm -f *~ lcheck
