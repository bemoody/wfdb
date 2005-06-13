# Note that this is NOT a relocatable package

Summary: Waveform Database library
Name: wfdb
Version: VERSION
Release: RPMRELEASE
License: GPL
Group: Libraries
Source0: http://www.physionet.org/physiotools/archives/wfdb-VERSION.tar.gz
Source1: http://www.physionet.org/physiotools/archives/wfdb-MAJOR.MINOR/wfdb-VERSION.tar.gz
URL: http://www.physionet.org/physiotools/wfdb.shtml
Vendor: PhysioNet
Packager: George Moody <george@mit.edu>
Requires: curl >= 7.10
Requires: curl-devel >= 7.10
BuildRoot: /var/tmp/%{name}-root

%changelog
* Wed Jun 8 2005 George B Moody <george@mit.edu>
- replaced libwww dependencies with libcurl

* Mon Mar 8 2004 George B Moody <george@mit.edu>
- added time2sec

* Wed Mar 19 2003 George B Moody <george@mit.edu>
- added --mandir to build, fixed linking in post

* Wed Dec 18 2002 George B Moody <george@mit.edu>
- split into wfdb, wfdb-devel, wfdb-app, wfdb-wave, wfdb-doc subpackages

* Sun Dec 8 2002 George B Moody <george@mit.edu>
- paths now use rpm's variables where possible

# ---- common prep/build/install/clean/post/postun ----------------------------

%prep
%setup

%build
PATH=$PATH:/usr/openwin/bin ./configure --prefix=/usr --mandir=%{_mandir}
make

# The 'make' command above actually *installs* the WFDB library and its *.h
# files in /usr/lib and /usr/include/wfdb/, because it is not possible
# otherwise to compile the apps and link them to the shared WFDB library.  As
# far as I can tell, there is no way to avoid this given the way RPM works.

# The 'make' commands below create HTML, PDF, and PostScript versions of the
# WFDB Programmer's Guide, WFDB Applications Guide, and WAVE User's Guide.
cd doc/wpg-src; make
cd ../wag-src; make
cd ../wug-src; make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/lib $RPM_BUILD_ROOT/usr/include/wfdb
mkdir -p $RPM_BUILD_ROOT/usr/bin
cd lib; cp -p libwfdb.so* $RPM_BUILD_ROOT/usr/lib
    cp -p wfdb.h ecgcodes.h ecgmap.h wfdblib.h $RPM_BUILD_ROOT/usr/include/wfdb
    cp -p wfdb-config $RPM_BUILD_ROOT/usr/bin
cd ../wave; make WFDBROOT=$RPM_BUILD_ROOT/usr install
cd ../waverc; make WFDBROOT=$RPM_BUILD_ROOT/usr install
cd ../app; make WFDBROOT=$RPM_BUILD_ROOT/usr install
cd ../psd; make WFDBROOT=$RPM_BUILD_ROOT/usr install
cd ../convert; make WFDBROOT=$RPM_BUILD_ROOT/usr install
cd ../data; make WFDBROOT=$RPM_BUILD_ROOT/usr install
cd ../doc; make WFDBROOT=$RPM_BUILD_ROOT/usr install

%clean
rm -rf $RPM_BUILD_ROOT
make clean

%post
/sbin/ldconfig
test -e /usr/lib/libwfdb.so.10 -a ! -e /usr/lib/libwfdb.so && ln -sf /usr/lib/libwfdb.so.10 /usr/lib/libwfdb.so

%postun -p /sbin/ldconfig

# ---- wfdb [shared library] package ------------------------------------------

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

%files
%defattr(-,root,root)
%{_libdir}/libwfdb.so*

# ---- wfdb-devel package -----------------------------------------------------

%package devel
Summary: WFDB developer's toolkit
Group: Development/Libraries
URL: http://www.physionet.org/physiotools/wpg/
Requires: wfdb = VERSION

%description devel
This package includes files needed to develop new WFDB applications in C, C++,
and Fortran, examples in C and in Fortran, and miscellaneous documentation.

%files devel
%defattr(-,root,root)
%{_bindir}/wfdb-config
%{_prefix}/database
%{_prefix}/include/wfdb
%doc checkpkg examples fortran lib/COPYING.LIB COPYING INSTALL MANIFEST NEWS README README.NETFILES

# ---- wfdb-app package -------------------------------------------------------

%package app
Summary: WFDB applications
Group: Applications/Scientific
URL: http://www.physionet.org/physiotools/wag/
Requires: wfdb >= VERSION

%description app
About 60 applications for creating, reading, transforming, analyzing,
annotating, and viewing digitized signals, especially physiologic signals.
Applications include digital filtering, event detection, signal averaging,
power spectrum estimation, and many others.

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
%{_bindir}/mit2wav
%{_bindir}/mrgann
%{_bindir}/mxm
%{_bindir}/nguess
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
%{_bindir}/time2sec
%{_bindir}/url_view
%{_bindir}/wabp
%{_bindir}/wav2mit
%{_bindir}/wfdbcat
%{_bindir}/wfdbcollate
%{_bindir}/wfdbdesc
%{_bindir}/wfdbwhich
%{_bindir}/wqrs
%{_bindir}/wrann
%{_bindir}/wrsamp
%{_bindir}/xform
%{_libdir}/ps
%{_mandir}

# ---- wfdb-wave package ------------------------------------------------------

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

WAVE uses the XView graphical user interface.

%files wave
%defattr(-,root,root)
%{_bindir}/wave
%{_bindir}/wave-remote
%{_bindir}/wavescript
%{_prefix}/help/
%config %{_prefix}/lib/wavemenu.def
%config %{_prefix}/lib/X11/app-defaults/Wave
%doc wave/anntab

# ---- wfdb-doc package -------------------------------------------------------

%package doc
Summary: WFDB documentation.
Group: Documentation
URL: http://www.physionet.org/physiotools/manuals.shtml

%description doc
This package includes HTML, PostScript, and PDF versions of the WFDB
Programmer's Guide, the WFDB Applications Guide, and the WAVE User's Guide.

%files doc
%defattr(-,root,root)
%doc doc/wag doc/wpg doc/wug


