#! /bin/sh
# file: fixpg.sh	G. Moody       12 April 1997
#			Last revised:  24 June 2002
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

cd $1

R=`grep -l "<TITLE>WFDB Programmer's Guide: WFDB path</TITLE>" *.htm`
if [ "x$R" != "x" ]; then ln -s $R database-path.htm
else echo "Can't find database-path.htm"
fi

R=`grep -l "<H2> Records" *.htm`
if [ "x$R" != "x" ]; then ln -s $R records.htm
else echo "Can't find records.htm"
fi

R=`grep -l "<TITLE>WFDB Programmer's Guide: strtim" *.htm`
if [ "x$R" != "x" ]; then ln -s $R strtim.htm
else echo "Can't find strtim.htm"
fi

