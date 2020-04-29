# This example works with either f77 (g77) or gfortran.
example$(EXEEXT):	example.f wfdbf.o
	$(F77) -o example$(EXEEXT) example.f wfdbf.o $(LDFLAGS)

wfdbf.o:	wfdbf.c
	$(CC) $(CFLAGS) -g -O -DFIXSTRINGS -c wfdbf.c


# If you have `f2c', but not `f77', use `make example-alt' instead of `make'.
example-alt:	example.f wfdbf.c
	f2c example.f
	$(CC) $(CFLAGS) -o example$(EXEEXT) example.c wfdbf.c \
	 -lf2c -lm $(LDFLAGS)


# 'make install' copies the wrapper sources into the directory where the
# WFDB headers are also installed.
install:
	../install.sh $(DESTDIR)$(INCDIR)/wfdb wfdbf.c
	$(SETPERMISSIONS) $(DESTDIR)$(INCDIR)/wfdb/wfdbf.c

collect:
	../conf/collect.sh $(INCDIR)/wfdb wfdbf.c

uninstall:
	../uninstall.sh $(DESTDIR)$(INCDIR)/wfdb wfdbf.c

clean:
	rm -f example$(EXEEXT) example.c *.o *~
