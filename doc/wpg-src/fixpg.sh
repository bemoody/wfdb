#! /bin/sh
# file: fixpg.sh	G. Moody       12 April 1997
#			Last revised:  21 July 2002
#
# Post-process WFDB Programmer's Guide HTML files

for i in $1/*.htm
do
  echo fixing links in $i ...
  cp $1/$i /tmp/.fix.$$
  L1=`wc -l <$1/$i`
  L2=`expr $L1 - 8`
  # discard and replace last 8 lines of each file
  head -$L2 </tmp/.fix.$$ >$1/$i
  cat >>$1/$i <<EOF
<p><address>
<a href="mailto:george@mit.edu"><em>George B. Moody
 (<tt>george@mit.edu</tt>)</em></A>
</address>
</body>
</html>
EOF
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
