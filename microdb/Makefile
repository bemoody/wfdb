# file: Makefile	G. Moody	7 February 1992
#			Last revised:	  7 May 1999
# UNIX `make' description file for `micro-database'
#
# This file is used with the UNIX `make' command to install the micro-database
# that comes with the WFDB software package.  Before using it for the first
# time, check that the site-specific variables below are appropriate for your
# system.  To install the micro-database, just type `make' (from within this
# directory).  To print a set of source listings, type `make listing'.

# Site-specific variables
# -----------------------

# DBDIR specifies the name of a directory in which to install the contents
# of the `microdb' directory.
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

install:
	cp 100s.* multi.hea null.hea $(DBDIR)
	-$(SETPERMISSIONS) $(DBDIR)/100s.* $(DBDIR)/multi.hea $(DBDIR)/null.hea

listing:
	$(PRINT) README Makefile makefile.dos
