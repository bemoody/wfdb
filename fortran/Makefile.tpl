
example:	example.f wfdbf.c
	f77 -o example -fwritable-strings example.f wfdbf.c -lwfdb

# If you have `f2c', but not `f77', use `make example-alt' instead of `make'.
example-alt:	example.f wfdbf.c
	f2c example.f
	$(CC) -o example example.c wfdbf.c -lf2c -lm -lwfdb

clean:
	rm -f example example.c *.o *~
