# file: Makefile	G. Moody	7 February 1992
#			Last revised:	4 September 1999
# UNIX `make' description file for WFDB calibration and sample data files
#
# -----------------------------------------------------------------------------
# WFDB data files
# Copyright (C) 1999 George B. Moody
#
# These files are free software; you can redistribute them and/or modify
# them under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# These programs are distributed in the hope that they will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# these programs; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# You may contact the author by e-mail (george@mit.edu) or postal mail
# (MIT Room E25-505A, Cambridge, MA 02139 USA).  For updates to this software,
# please visit the author's web site (http://ecg.mit.edu/).
# _____________________________________________________________________________
#
# This file is used with the UNIX `make' command to install the calibration and
# sample database files that come with the WFDB software package.  Before using
# it for the first time, check that the site-specific variables below are
# appropriate for your system.  To install these files, just type `make' (from
# within this directory).  To print a set of source listings, type `make
# listing'.

# Site-specific variables
# -----------------------

# DBDIR specifies the name of a directory in which to install the contents
# of this directory.
DBDIR = /usr/local/database

# PRINT is the name of the program used to produce listings (including any
# options for the desired formatting).
PRINT = lpr

# SETPERMISSIONS is the command needed to make the installed files accessible
# to those who will use them.  The value given below makes them readable by
# everyone, and writeable by the owner only.  (If you perform the installation
# as `root', `root' is the owner of the installed files.)
SETPERMISSIONS = chmod 644

# It should not be necessary to modify anything below this line.
# -----------------------------------------------------------------------------

DBFILES = 100s.dat 100s.atr *.hea *list wfdbcal

install:	$(DBDIR)
	cp $(DBFILES) $(DBDIR)
	-cd $(DBDIR); $(SETPERMISSIONS) $(DBFILES)
	-cd $(DBDIR); ln -sf wfdbcal dbcal

all:
	@echo Nothing to be made in `pwd`.

$(DBDIR):
	-mkdir $(DBDIR)

listing:
	$(PRINT) README Makefile makefile.dos

clean:
	rm -f *~
