#! /bin/sh
# file: fixag.sh	G. Moody	10 April 1997
#			Last revised:	25 May 1999
#
# Post-process WFDB Applications Guide HTML files

for i in $*
do
  F=`basename $i .html`
  G=`echo $F | sed 's/\./-/g'`
  echo -n processing $F ...
  sed 's/\.html/\.htm/g' <$F.html |
  sed 's/evfootnode\.htm/evfoot\.htm/g' |
  sed 's/\.\(.\)\.htm/-\1.htm/g' >$G.htm
  echo " done"
done
mv -f evfootnode.htm evfoot.htm

for i in *.htm
do
  echo -n fixing links in $i ...
  cp $i .fix.$$
  sed -f fixag.sed <.fix.$$ >$i
  echo " done"
done
rm .fix.$$
