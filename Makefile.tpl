# file: Makefile.tpl		G. Moody		24 May 2000
#				Last revised:		30 May 2001
# This section of the Makefile should not need to be changed.

# ARCH specifies the type of CPU and the operating system (e.g., 'i686-Linux').
# This symbol is used only to generate a name for the binary archive;  it
# does not affect how the software is compiled.
ARCH=`uname -m`-`uname -s`

# 'make' or 'make all': compile the WFDB applications without installing them
# (requires installation of the WFDB library and includes)
all:		config.cache
	cd lib;	     $(MAKE) install
	cd wave;     $(MAKE) all
	cd waverc;   $(MAKE) all
	cd app;      $(MAKE) all
	cd psd;      $(MAKE) all
	cd examples; $(MAKE) all
	cd convert;  $(MAKE) all

# 'make install': compile and install the WFDB software package
install:	config.cache
	cd lib;	     $(MAKE) install
	cd wave;     $(MAKE) install
	cd waverc;   $(MAKE) install
	cd app;      $(MAKE) install
	cd psd;      $(MAKE) install
	cd convert;  $(MAKE) install
	cd data;     $(MAKE) install

uninstall:	config.cache
	cd lib;	     $(MAKE) uninstall
	cd wave;     $(MAKE) uninstall
	cd waverc;   $(MAKE) uninstall
	cd app;      $(MAKE) uninstall
	cd psd;      $(MAKE) uninstall
	cd convert;  $(MAKE) uninstall
	cd data;     $(MAKE) uninstall
	./uninstall.sh $(WFDBROOT)

# 'make clean': remove binaries, other cruft from source directories
clean:
	cd app;      $(MAKE) clean
	cd convert;  $(MAKE) clean
	cd data;     $(MAKE) clean
	cd doc;      $(MAKE) clean
	cd examples; $(MAKE) clean
	cd fortran;  $(MAKE) clean
	cd lib;      $(MAKE) clean
	cd psd;      $(MAKE) clean
	cd wave;     $(MAKE) clean
	cd wave-doc; $(MAKE) clean
	cd waverc;   $(MAKE) clean
	cd wview;    $(MAKE) -f clean
	rm -f *~ conf/*~ config.cache */*.exe

# 'make config.cache': check configuration
config.cache:
	exec ./configure
	@echo "(Ignore any error that may appear on the next line.)"
	@false	# force an immediate exit from `make'

# 'make test' or 'make test-all': compile the WFDB applications without
# installing them (installs the dynamically-linked WFDB library and includes
# into subdirectories of $(HOME)/wfdb-test)
test test-all: $(HOME)/wfdb-test/include $(HOME)/wfdb-test/lib
	$(MAKE) WFDBROOT=$(HOME)/wfdb-test all

# 'make test-install': compile and install the WFDB software package into
# subdirectories of $(HOME)/wfdb-test
test-install: $(TESTDIRS)
	$(MAKE) WFDBROOT=$(HOME)/wfdb-test install

# Create directories for test installation if necessary.
TESTDIRS = $(HOME)/wfdb-test/bin $(HOME)/wfdb-test/database \
 $(HOME)/wfdb-test/help $(HOME)/wfdb-test/include $(HOME)/wfdb-test/lib

$(HOME)/wfdb-test:
	mkdir -p $(HOME)/wfdb-test; $(SETDPERMISSIONS) $(HOME)/wfdb-test
$(HOME)/wfdb-test/bin:		$(HOME)/wfdb-test
	mkdir -p $(HOME)/wfdb-test/bin; \
	 $(SETDPERMISSIONS) $(HOME)/wfdb-test/bin
$(HOME)/wfdb-test/database:	$(HOME)/wfdb-test
	mkdir -p $(HOME)/wfdb-test/database; \
	 $(SETDPERMISSIONS) $(HOME)/wfdb-test/database
$(HOME)/wfdb-test/help:		$(HOME)/wfdb-test
	mkdir -p $(HOME)/wfdb-test/help; \
	 $(SETDPERMISSIONS) $(HOME)/wfdb-test/help
$(HOME)/wfdb-test/include:	$(HOME)/wfdb-test
	mkdir -p $(HOME)/wfdb-test/include; \
	 $(SETDPERMISSIONS) $(HOME)/wfdb-test/include
$(HOME)/wfdb-test/lib:		$(HOME)/wfdb-test
	mkdir -p $(HOME)/wfdb-test/lib; \
	 $(SETDPERMISSIONS) $(HOME)/wfdb-test/lib

# 'make tarballs': clean up the source directories, then make a pair of gzipped
# tar source archives of the WFDB software package (with and without the
# documentation), and check that the MANIFEST (list of files in the package)
# is correct.
tarballs:	clean
	cd ..; tar --create --file $(PACKAGE).tar.gz --verbose --gzip \
          '--exclude=$(PACKAGE)/*CVS' $(PACKAGE) | tee $(PACKAGE)-MANIFEST
	cd ..; tar --create --file $(PACKAGE)-no-docs.tar.gz \
	  --verbose --gzip \
          '--exclude=$(PACKAGE)/*doc' \
	  '--exclude=$(PACKAGE)/*CVS' $(PACKAGE)
	./check-manifest $(PACKAGE)

# 'make bin-tarball': make a gzipped tar archive of the WFDB software package
# binaries and other installed files
bin-tarball:	test-install
	cd $(HOME)/wfdb-test;  tar cfvz ../$(PACKAGE)-$(ARCH).tar.gz .
