#! /bin/sh
# file: fixpg.sh	G. Moody       12 April 1997
#			Last revised:    1 May 2020
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
<a href="mailto:wfdb@physionet.org"><em>PhysioNet
 (<tt>wfdb@physionet.org</tt>)</em></A>
</address>
</body>
</html>
EOF
done

cp $1/wpg_toc.htm /tmp/.fix.$$
sed "s+WFDB Database Programmer's Guide<+<A HREF=\"wpg.htm\">WFDB Programmer's Guide</A><+" </tmp/.fix.$$ >$1/wpg_toc.htm
rm /tmp/.fix.$$

cd $1

for L in database-path.htm records.htm strtim.htm compiling.htm
do
  rm -f "$L"
  R=`grep -l "link: $L" *.htm`
  if [ -f "$R" ]; then ln -s "$R" "$L"
  else echo "Can't find $L"; exit 1
  fi
done
