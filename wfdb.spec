# Note that this is NOT a relocatable package

Summary: Waveform Database library
Name: wfdb
Version: VERSION
Release: RPMRELEASE
License: GPL
Group: Libraries
Source: http://www.physionet.org/physiotools/archives/wfdb-VERSION.tar.gz
URL: http://www.physionet.org/physiotools/wfdb.shtml
Packager: George Moody <george@mit.edu>
Requires: w3c-libwww >= 5.2
Requires: w3c-libwww-devel >= 5.2
BuildRoot: /var/tmp/%{name}-root

%changelog
* Wed Dec 18 2002 George B Moody <george@mit.edu>
- split into wfdb, wfdb-devel, wfdb-app, wfdb-doc, wfdb-wave packages

* Sun Dec 8 2002 George B Moody <george@mit.edu>
- paths now use rpm's variables where possible

%description
The WFDB (Waveform Database) library supports creating, reading, and annotating
digitized signals in a wide variety of formats.  Input can be from local files
or directly from web or FTP servers (via the W3C's libwww).  WFDB applications
need not be aware of the source or format of their input, since input files are
located by searching a path that may include local and remote components, and
all data are transparently converted on-the-fly into a common format.  Although
created for use with physiologic signals such as those in PhysioBank
(http://www.physionet.org/physiobank/), the WFDB library supports a broad
range of general-purpose signal processing applications.

%package devel
Summary: WFDB developer's toolkit
Group: Development/Libraries
Requires: wfdb = VERSION

%description devel
This package includes files needed to develop new WFDB applications in C, C++,
and Fortran, examples in C and in Fortran, and miscellaneous documentation.

%package app
Summary: WFDB applications.
Group: Applications/Scientific
URL: http://www.physionet.org/physiotools/wag/
Requires: wfdb >= VERSION

%description app
About 60 applications for creating, reading, transforming, analyzing,
annotating, and viewing digitized signals, especially physiologic signals.
Applications include digital filtering, event detection, signal averaging,
power spectrum estimation, and many others.

%package wave
Summary: Waveform Analyzer, Viewer, and Editor.
Group: X11/Applications/Science
URL: http://www.physionet.org/physiotools/wug/
Requires: wfdb >= VERSION
Requires: wfdb-app
Requires: xview >= 3.2
Requires: xview-devel >= 3.2

%description wave
WAVE provides an environment for exploring digitized signals and time series.
It provides fast, high-quality views of data stored locally or on remote
web or FTP servers, flexible control of standard and user-provided analysis
modules, efficient interactive annotation editing, and support for multiple
views on the same or different displays to support collaborative analysis and
annotation projects.  WAVE has been used to develop annotations for most of
the PhysioBank databases (http://www.physionet.org/physiobank/).

WAVE uses the XView graphical user interface.  A (beta) version of WAVE that
uses GTK+ is available at http://www.physionet.org/physiotools/beta/gtkwave/.

#%package doc
#Summary: WFDB documentation.
#Group: Documentation
#URL: http://www.physionet.org/physiotools/manuals.shtml
#
#%description doc
#This package includes HTML, PostScript, and PDF versions of the WFDB
#Programmer's Guide, the WFDB Applications Guide, and the WAVE User's Guide.

%prep
%setup
PATH=$PATH:/usr/openwin/bin ./configure --prefix=%{_prefix}

%build
make
#( cd doc/wag-src; make )
#( cd doc/wpg-src; make )
#( cd doc/wug-src; make )

%install
rm -rf $RPM_BUILD_ROOT
make WFDBROOT=$RPM_BUILD_ROOT/usr install
mkdir -p $RPM_BUILD_ROOT/usr/lib/X11/app-defaults
cp -p /usr/lib/wavemenu.def $RPM_BUILD_ROOT/usr/lib
cp -p /usr/lib/X11/app-defaults/Wave $RPM_BUILD_ROOT/usr/lib/X11/app-defaults
mkdir -p $RPM_BUILD_ROOT/usr/lib/ps
cp -pr /usr/lib/ps/pschart.pro /usr/lib/ps/12lead.pro /usr/lib/ps/psfd.pro \
 $RPM_BUILD_ROOT/usr/lib/ps
mkdir -p $RPM_BUILD_ROOT/usr/help
cp -pr /usr/help/wave $RPM_BUILD_ROOT/usr/help

%clean
rm -rf $RPM_BUILD_ROOT
make clean

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%{_libdir}/libwfdb.so*

%files devel
%{_bindir}/wfdb-config
%{_prefix}/include/wfdb
%doc checkpkg examples fortran lib/COPYING.LIB COPYING INSTALL MANIFEST NEWS README README.NETFILES

%files app
%defattr(-,root,root)
%{_bindir}/a2m
%{_bindir}/ad2m
%{_bindir}/ahaconvert
%{_bindir}/ann2rr
%{_bindir}/bxb
%{_bindir}/calsig
%{_bindir}/coherence
%{_bindir}/cshsetwfdb
%{_bindir}/ecgeval
%{_bindir}/edf2mit
%{_bindir}/epicmp
%{_bindir}/fft
%{_bindir}/fir
%{_bindir}/hrfft
%{_bindir}/hrlomb
%{_bindir}/hrmem
%{_bindir}/hrplot
%{_bindir}/ihr
%{_bindir}/log10
%{_bindir}/lomb
%{_bindir}/m2a
%{_bindir}/makeid
%{_bindir}/md2a
%{_bindir}/memse
%{_bindir}/mfilt
%{_bindir}/mit2edf
%{_bindir}/mrgann
%{_bindir}/mxm
%{_bindir}/nst
%{_bindir}/plot2d
%{_bindir}/plot3d
%{_bindir}/plotstm
%{_bindir}/pscgen
%{_bindir}/pschart
%{_bindir}/psfd
%{_bindir}/rdann
%{_bindir}/rdsamp
%{_bindir}/readid
%{_bindir}/revise
%{_bindir}/rr2ann
%{_bindir}/rxr
%{_bindir}/sampfreq
%{_bindir}/setwfdb
%{_bindir}/sigamp
%{_bindir}/sigavg
%{_bindir}/skewedit
%{_bindir}/snip
%{_bindir}/sortann
%{_bindir}/sqrs
%{_bindir}/sqrs125
%{_bindir}/sumann
%{_bindir}/sumstats
%{_bindir}/tach
%{_bindir}/url_view
%{_bindir}/wabp
%{_bindir}/wfdbcat
%{_bindir}/wfdbcollate
%{_bindir}/wfdbdesc
%{_bindir}/wfdbwhich
%{_bindir}/wqrs
%{_bindir}/wrann
%{_bindir}/wrsamp
%{_bindir}/xform
%{_prefix}/database
/usr/lib/ps/12lead.pro
/usr/lib/ps/pschart.pro
/usr/lib/ps/psfd.pro
%{_prefix}/local/man

%files wave
%defattr(-,root,root)
%{_bindir}/wave
%{_bindir}/wave-remote
%{_bindir}/wavescript
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
%config /usr/lib/wavemenu.def
%config /usr/lib/X11/app-defaults/Wave
%doc wave/anntab

#%files doc
#%defattr(-,root,root)
#%doc doc/wag doc/wpg doc/wug
