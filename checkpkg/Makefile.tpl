all:	libcheck
	@echo Testing the WFDB library ...
	@./libcheck -v 2>&1 | grep -v Checking >libcheck.log
	@./appcheck

libcheck:	libcheck.c
	$(CC) $(CFLAGS) libcheck.c -o $@ $(LDFLAGS)

clean:
	rm -f *~ libcheck
