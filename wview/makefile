# file: makefile	G. Moody	9 March 1993
#			Last revised:	 7 May 1999
# MSDOS `make' description file template for compiling WVIEW
#
# Copyright (C) 1999 George B. Moody
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# You may contact the author by e-mail (george@mit.edu) or postal mail
# (MIT Room E25-505A, Cambridge, MA 02139 USA).  For updates to this software,
# please visit the author's web site (http://ecg.mit.edu/).
# _____________________________________________________________________________

# The value of INCDIR should match that in the makefile for the WFDB library.
INCDIR = c:\include

# Uncomment the next line if you are using Microsoft C/C++.
#MSC =

!ifdef MSC
CC = cl
CFLAGS = -nologo -G2sw -Zp -W3 -AL -Os -D_WINDOWS -I$(INCDIR)
LIBS = libw llibcew ldllcew wfdbdll
LINK = link /NOD /NOE
MAPFILE = wview.map
!endif

# If you are using neither Microsoft nor Borland C/C++, insert appropriate
# definitions for CC, CFLAGS, LIBS, and LINK here.

# Otherwise, this makefile assumes you are using Borland C/C++.
!ifndef LIBS
CC = bcc
CFLAGS = -2 -WE -w- -ml -Os -D_WINDOWS -I$(INCDIR)
LIBS = twfdbdll import mathwl cwl
LINK = tlink /Tw/c/x c0wl
MAPFILE = 
!endif

.c.obj:
    $(CC) $(CFLAGS) -c $*.c

.rc.res:
    rc -r $*.rc

wview.exe:	wview.obj wview.def wview.res
    $(LINK) wview, wview, $(MAPFILE), $(LIBS), wview.def
    rc -T wview.res

wview.obj:	wview.h
wview.res:	wview.rc

wview.hlp: wview.hpj wview.rtf
	hc31 wview
	del wview.ph

clean:
    -del *.obj
    -del *.res
    -del *.sym
    -del *.map
    -del *.~
    -del *.c~
    -del *.de~
    -del *.h~
    -del *.hp~
    -del *.rc~
    -del *.rt~

