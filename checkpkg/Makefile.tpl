all:	lcheck
	@./libcheck
	@./appcheck

lcheck:	lcheck.c
	$(CC) $(CFLAGS) lcheck.c -o $@ $(LDFLAGS)

clean:
	rm -f *~ lcheck
