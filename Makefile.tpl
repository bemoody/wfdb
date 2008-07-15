# file: Makefile.tpl		G. Moody	 24 May 2000
#				Last revised:    11 May 2006
# This section of the Makefile should not need to be changed.

# 'make' or 'make all': compile the WFDB applications without installing them
all:		config.cache
	$(MAKE) WFDBROOT=`pwd`/build install check

# 'make install': compile and install the WFDB software package
install:	config.cache
	cd lib;	     $(MAKE) install
	cd app;      $(MAKE) install
	cd convert;  $(MAKE) install
	cd data;     $(MAKE) install
	cd fortran;  $(MAKE) install
	cd psd;      $(MAKE) install
	-( cd wave;  $(MAKE) install )
	cd waverc;   $(MAKE) install
	test -d doc && ( cd doc; $(MAKE) install )

# 'make collect': collect the installed files into /tmp/wfdb/
collect:
	cd lib;	     $(MAKE) collect
	cd app;      $(MAKE) collect
	cd convert;  $(MAKE) collect
	cd data;     $(MAKE) collect
	cd fortran;  $(MAKE) collect
	cd psd;      $(MAKE) collect
	-( cd wave;  $(MAKE) collect )
	cd waverc;   $(MAKE) collect
	test -d doc && ( cd doc; $(MAKE) collect )

uninstall:	config.cache
	cd app;      $(MAKE) uninstall
	cd convert;  $(MAKE) uninstall
	cd data;     $(MAKE) uninstall
	cd fortran;  $(MAKE) uninstall
	cd lib;	     $(MAKE) uninstall
	cd psd;      $(MAKE) uninstall
	cd wave;     $(MAKE) uninstall
	cd waverc;   $(MAKE) uninstall
	test -d doc && ( cd doc; $(MAKE) uninstall )
	./uninstall.sh $(WFDBROOT)

# 'make clean': remove binaries, other cruft from source directories
clean:
	cd app;      $(MAKE) clean
	cd checkpkg; $(MAKE) clean
	cd convert;  $(MAKE) clean
	cd data;     $(MAKE) clean
	cd examples; $(MAKE) clean
	cd fortran;  $(MAKE) clean
	cd lib;      $(MAKE) clean
	cd psd;      $(MAKE) clean
	cd wave;     $(MAKE) clean
	cd waverc;   $(MAKE) clean
	test -d doc && ( cd doc; $(MAKE) clean )
	cd conf; rm -f *~ prompt site.def site-slib.def
	rm -f *~ config.cache */*.exe $(PACKAGE)-*.spec
	rm -rf build

# 'make config.cache': check configuration
config.cache:
	exec ./configure
	@echo "(Ignore any error that may appear on the next line.)"
	@false	# force an immediate exit from `make'

conf/prompt:
	echo -n >echo.out
	-test -s echo.out && ln -sf prompt-c conf/prompt
	-test -s echo.out || ln -sf prompt-n conf/prompt
	rm echo.out

# 'make test' or 'make test-all': compile the WFDB applications without
# installing them (installs the dynamically-linked WFDB library and includes
# into subdirectories of $(HOME)/wfdb-test)
test test-all: $(HOME)/wfdb-test/include $(HOME)/wfdb-test/lib
	$(MAKE) WFDBROOT=$(HOME)/wfdb-test all

# 'make test-install': compile and install the WFDB software package into
# subdirectories of $(HOME)/wfdb-test
test-install: $(TESTDIRS)
	$(MAKE) WFDBROOT=$(HOME)/wfdb-test install

# 'make check': test currently installed version of the WFDB software package
check:		config.cache conf/prompt
	cd checkpkg; $(MAKE) all

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
	rm -f ../$(PACKAGE)-MANIFEST ../$(PACKAGE).tar.gz \
	  ../$(PACKAGE)-no-docs.tar.gz
	cd lib; $(SETPERMISSIONS) *.h
	cd ..; tar --create --file $(PACKAGE).tar.gz --verbose --gzip \
          '--exclude=$(PACKAGE)/*CVS' $(PACKAGE) | sed s+${PACKAGE}/++ | \
	  tee $(PACKAGE)-MANIFEST
	cd ..; tar --create --file $(PACKAGE)-no-docs.tar.gz \
	  --verbose --gzip \
          '--exclude=$(PACKAGE)/*doc' \
	  '--exclude=$(PACKAGE)/*CVS' $(PACKAGE)
	./check-manifest $(PACKAGE)

# 'make bin-tarball': make a gzipped tar archive of the WFDB software package
# binaries and other installed files
bin-tarball:	install collect
	cp conf/archname /tmp; chmod +x /tmp/archname
	rm -rf /tmp/$(PACKAGE)-`/tmp/archname`
	mv /tmp/wfdb /tmp/$(PACKAGE)-`/tmp/archname`
	cd /tmp; tar cfvz $(PACKAGE)-`/tmp/archname`.tar.gz \
	 $(PACKAGE)-`/tmp/archname`
	mv /tmp/$(PACKAGE)-`/tmp/archname`.tar.gz ..
	rm -rf /tmp/$(PACKAGE)-`/tmp/archname`
	rm -f /tmp/archname

# 'make doc-tarball': make a gzipped tar archive of formatted documents
# (requires many freely-available utilities that are not part of this
# package;  see doc/Makefile.top for details)
doc-tarball:
	cd doc; $(MAKE) tarball

# 'make rpms': make source and binary RPMs
RPMROOT=/usr/src/redhat

rpms:		tarballs
	cp -p ../$(PACKAGE).tar.gz $(RPMROOT)/SOURCES
	sed s/VERSION/$(VERSION)/g <wfdb.spec | \
	 sed s/MAJOR/$(MAJOR)/g | sed s/MINOR/$(MINOR)/g | \
	 sed s/RPMRELEASE/$(RPMRELEASE)/ >$(PACKAGE)-$(RPMRELEASE).spec
	cp -p $(PACKAGE)-$(RPMRELEASE).spec $(RPMROOT)/SPECS
	if [ -x /usr/bin/rpmbuild ]; \
	 then rpmbuild -ba $(PACKAGE)-$(RPMRELEASE).spec; \
	 else echo "rpmbuild not found in /usr/bin; attempting to use rpm"; \
	  rpm -ba $(PACKAGE)-$(RPMRELEASE).spec; fi
	mv $(RPMROOT)/RPMS/*/wfdb*-$(VERSION)-$(RPMRELEASE).*.rpm ..
	mv $(RPMROOT)/SRPMS/$(PACKAGE)-$(RPMRELEASE).src.rpm ..
	rm -f $(PACKAGE)-$(RPMRELEASE).spec
	echo "Remember to sign the RPMs by"
	echo "   cd ..; rpm --addsign wfdb*$(VERSION)*rpm"