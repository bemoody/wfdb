all:
	@rm -f lcheck
	@make lcheck	
	@./libcheck $(DBDIR)
	@../conf/prompt "Press <Enter> to continue ... "
	@read a
	@./appcheck

lcheck:	lcheck.c
	@echo Compiling WFDB library test application ...
	@$(CC) $(CFLAGS) lcheck.c -o $@ $(LDFLAGS) && echo " Succeeded"

clean:
	rm -f *~ lcheck
