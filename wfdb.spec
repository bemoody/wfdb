Summary: Waveform Database library and applications
Name: wfdb
Version: VERSION
Release: RPMRELEASE
Copyright: GPL
Group: Applications/Engineering
Source: http://www.physionet.org/physiotools/archives/wfdb-VERSION.tar.gz
URL: http://www.physionet.org/physiotools/wfdb.shtml
Packager: George Moody <george@mit.edu>
Requires: w3c-libwww >= 5.2
Requires: w3c-libwww-devel >= 5.2
Requires: xview >= 3.2
Requires: xview-devel >= 3.2

%description
Applications for creating, reading, analyzing, and viewing digitized signals in
a wide variety of formats, with optional annotations.  The WFDB library,
included here, works together with the W3C's libwww to provide HTTP and FTP
client support to applications that use it, such as those in this package.
Although created for use with physiologic signals such as those available from
PhysioBank (http://www.physionet.org/physiobank/), many of the programs in this
package are general-purpose signal processing applications.

%prep
%setup
PATH=$PATH:/usr/openwin/bin ./configure --prefix=/usr

%build
make

%install
make install

%clean
make clean

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-,root,root)
%doc checkpkg doc examples fortran lib/COPYING.LIB wave/anntab wave/wavemenu.def COPYING INSTALL MANIFEST NEWS README README.NETFILES

/usr/bin/a2m
/usr/bin/ad2m
/usr/bin/ahaconvert
/usr/bin/ann2rr
/usr/bin/bxb
/usr/bin/calsig
/usr/bin/coherence
/usr/bin/cshsetwfdb
/usr/bin/ecgeval
/usr/bin/edf2mit
/usr/bin/epicmp
/usr/bin/fft
/usr/bin/fir
/usr/bin/hrfft
/usr/bin/hrlomb
/usr/bin/hrmem
/usr/bin/hrplot
/usr/bin/ihr
/usr/bin/log10
/usr/bin/lomb
/usr/bin/m2a
/usr/bin/makeid
/usr/bin/md2a
/usr/bin/memse
/usr/bin/mfilt
/usr/bin/mit2edf
/usr/bin/mrgann
/usr/bin/mxm
/usr/bin/nst
/usr/bin/plot2d
/usr/bin/plot3d
/usr/bin/plotstm
/usr/bin/pscgen
/usr/bin/pschart
/usr/bin/psfd
/usr/bin/rdann
/usr/bin/rdsamp
/usr/bin/readid
/usr/bin/revise
/usr/bin/rr2ann
/usr/bin/rxr
/usr/bin/sampfreq
/usr/bin/setwfdb
/usr/bin/sigamp
/usr/bin/sigavg
/usr/bin/skewedit
/usr/bin/snip
/usr/bin/sortann
/usr/bin/sqrs
/usr/bin/sqrs125
/usr/bin/sumann
/usr/bin/sumstats
/usr/bin/tach
/usr/bin/url_view
/usr/bin/wave
/usr/bin/wave-remote
/usr/bin/wavescript
/usr/bin/wfdbcat
/usr/bin/wfdbcollate
/usr/bin/wfdb-config
/usr/bin/wfdbdesc
/usr/bin/wfdbwhich
/usr/bin/wqrs
/usr/bin/wrann
/usr/bin/wrsamp
/usr/bin/xform
/usr/database/100s.atr
/usr/database/100s.dat
/usr/database/100s.hea
/usr/database/16.hea
/usr/database/16l.hea
/usr/database/8.hea
/usr/database/8l.hea
/usr/database/ahalist
/usr/database/ahaxlist
/usr/database/culist
/usr/database/dbcal
/usr/database/dblist
/usr/database/esclist
/usr/database/mitlist
/usr/database/mitxlist
/usr/database/multi.hea
/usr/database/nstlist
/usr/database/null.hea
/usr/database/pipe/16x10.hea
/usr/database/pipe/16x11.hea
/usr/database/pipe/16x12.hea
/usr/database/pipe/16x13.hea
/usr/database/pipe/16x14.hea
/usr/database/pipe/16x15.hea
/usr/database/pipe/16x16.hea
/usr/database/pipe/16x1.hea
/usr/database/pipe/16x2.hea
/usr/database/pipe/16x3.hea
/usr/database/pipe/16x4.hea
/usr/database/pipe/16x5.hea
/usr/database/pipe/16x6.hea
/usr/database/pipe/16x7.hea
/usr/database/pipe/16x8.hea
/usr/database/pipe/16x9.hea
/usr/database/pipe/8x10.hea
/usr/database/pipe/8x11.hea
/usr/database/pipe/8x12.hea
/usr/database/pipe/8x13.hea
/usr/database/pipe/8x14.hea
/usr/database/pipe/8x15.hea
/usr/database/pipe/8x16.hea
/usr/database/pipe/8x1.hea
/usr/database/pipe/8x2.hea
/usr/database/pipe/8x3.hea
/usr/database/pipe/8x4.hea
/usr/database/pipe/8x5.hea
/usr/database/pipe/8x6.hea
/usr/database/pipe/8x7.hea
/usr/database/pipe/8x8.hea
/usr/database/pipe/8x9.hea
/usr/database/tape/10240.hea
/usr/database/tape/1024.hea
/usr/database/tape/4096.hea
/usr/database/tape/512.hea
/usr/database/tape/6144d.hea
/usr/database/tape/ahatape.hea
/usr/database/tape/mittape.hea
/usr/database/wfdbcal
/usr/help/wave/analysis.hlp
/usr/help/wave/buttons.hlp
/usr/help/wave/demo.txt
/usr/help/wave/editing.hlp
/usr/help/wave/intro.hlp
/usr/help/wave/log.hlp
/usr/help/wave/news.hlp
/usr/help/wave/printing.hlp
/usr/help/wave/resource.hlp
/usr/help/wave/wave.hlp
/usr/help/wave/wave.info
/usr/help/wave/wave.pro
/usr/include/wfdb/ecgcodes.h
/usr/include/wfdb/ecgmap.h
/usr/include/wfdb/wfdb.h
/usr/lib/libwfdb.so.10.2
/usr/lib/ps/12lead.pro
/usr/lib/ps/pschart.pro
/usr/lib/ps/psfd.pro
/usr/lib/wavemenu.def
/usr/lib/X11/app-defaults/Wave
