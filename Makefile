all:
	cd lib; make
	cd lib; make slib
	cd wave; make
	cd waverc; make
	cd app; make
	cd psd; make
	cd examples; make
	cd convert; make

clean:
	cd lib; make clean
	cd wave; make clean
	cd waverc; make clean
	cd app; make clean
	cd psd; make clean
	cd examples; make clean
	cd convert; make clean
	rm -f *~

tarballs:	clean
	cd ..; tar cfvz wfdb.tar.gz wfdb
	cd ..; tar --create --file wfdb-no-docs.tar.gz --verbose --gzip '--exclude=wfdb/*doc' wfdb
