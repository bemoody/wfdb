# file: Makefile.tpl		G. Moody		23 May 2000
#				Last revised:		5 June 2000
# This section of the Makefile should not need to be changed.

CFILES = psamples.c exgetvec.c exputvec.c exannstr.c example1.c example2.c \
 example3.c example4.c example5.c example6.c example7.c example8.c example9.c \
 example10.c refhr.c
XFILES = psamples exgetvec exputvec exannstr example1 example2 example3 \
 example4 example5 example6 example7 example8 example9 example10 refhr
MFILES = Makefile makefile.dos

# General rule for compiling C sources into executable files.  This is
# redundant for most versions of `make', but at least one System V version
# needs it.
.c:
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# `make' or `make all':  compile the examples
all:	$(XFILES)

# `make install':  compile but do no more
install:	all

uninstall:
	echo "Nothing to be done for uninstall"

# `make listing':  print a listing of sources
listing:
	$(PRINT) README $(MFILES) $(CFILES)

# `make clean':  remove executable, intermediate and backup files
clean:
	rm -f $(XFILES) *.o *~ core

# Odds and ends.
example10.c:
	mv exampl10.c example10.c
