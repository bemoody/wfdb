#! /bin/sh
# file: fixpg.sh	G. Moody	12 April 1997
#			Last revised:  20 December 2001
#
# Post-process WFDB Programmer's Guide HTML files

for i in $1/*.htm
do
  echo fixing links in $i ...
  cp $1/$i /tmp/.fix.$$
  sed -f fixpg.sed </tmp/.fix.$$ >$1/$i
done

cp $1/wpg_toc.htm /tmp/.fix.$$
sed "s+WFDB Database Programmer's Guide<+<A HREF=\"wpg.htm\">WFDB Programmer's Guide</A><+" </tmp/.fix.$$ >$1/wpg_toc.htm
rm /tmp/.fix.$$
