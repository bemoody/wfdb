#! /bin/sh
# file: fixpg.sh	G. Moody	12 April 1997
#			Last revised:	25 May 1999
#
# Post-process WFDB Programmer's Guide HTML files

for i in $1/*.htm
do
  echo -n fixing links in $i ...
  cp $1/$i /tmp/.fix.$$
  sed -f fixpg.sed </tmp/.fix.$$ >$1/$i
  echo " done"
done

cp $1/dbu_toc.htm /tmp/.fix.$$
sed "s+WFDB Database Programmer's Guide<+<A HREF=\"dbpg.htm\">WFDB Programmer's Guide</A><+" </tmp/.fix.$$ >$1/dbu_toc.htm
rm /tmp/.fix.$$
