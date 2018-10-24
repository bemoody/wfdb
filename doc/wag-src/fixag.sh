#! /bin/sh
# file: fixag.sh	G. Moody	10 April 1997
#			Last revised:   24 October 2018
#
# Post-process WFDB Applications Guide HTML files

set -e

URLPREFIX=http://www.physionet.org/physiotools/wag/

LONGDATE=$1
shift

for i in $*
do
  F=`basename $i .html`
  G=`echo $F | sed 's/\./-/g'`
  echo processing $F ...
  sed 's/\.html/\.htm/g' <$F.html |
  sed 's/evfootnode\.htm/evfoot\.htm/g' |
  sed 's/<BODY >/<BODY BGCOLOR="#FFFFFF">/' |
  sed 's/\.\(.\)\.htm/-\1.htm/g' >$G.htm
done
mv -f evfootnode.htm evfoot.htm

for i in *.htm
do
  echo fixing links in $i ...
  cp $i .fix.$$
  sed -f fixag.sed <.fix.$$ | sed "s/LONGDATE/$LONGDATE/" |
    sed "s+PAGENAME+$URLPREFIX$i+" >$i
done
rm .fix.$$

PREVT=
PREVU=
THIST=FAQ
THISU="faq.htm"
NEXTT=
NEXTU=

for i in *-1.htm *-3.htm *-5.htm *-7.htm
do
  NEXTT=`grep "<TITLE>" $i | sed "s+<TITLE>++" | sed "s+</TITLE>++"`
  if [ "x$NEXTT" = "x" ]
  then
    NEXTT=`grep "<title>" $i | sed "s+<title>++" | sed "s+</title>++"`
  fi
  NEXTU=$i
  if [ "$THISU" = "faq.htm" ]
  then
    sed "s+NEXTPAGE+<a href=$NEXTU>$NEXTT</a>+" <$THISU >tmp.$$
  else
    sed "s+NEXTPAGE+<a href=$NEXTU>$NEXTT</a>+" <$THISU |
     sed "s+>record+><a href=\"intro.htm#record\">record</a>+g" |
     sed "s+>annotator+><a href=\"intro.htm#annotator\">annotator</a>+g" |
     sed "s+[a-zA-Z]*-annotator+<a href=\"intro.htm#annotator\">&</a>+g" |
     sed "s+>\\(ann[1-3]\\)\\([ <]\\)+><a href=\"intro.htm#annotator\">\\1</a>\\2+g" |
     sed "s+>time+><a href=\"intro.htm#time\">time</a>+g" |
     sed "s+>signal-list+><a href=\"intro.htm#signal-list\">signal-list</a>+g" |
     sed "s+>signal</i+><a href=\"intro.htm#signal\">signal</a></i+g" |
     sed "s+PREVPAGE+<a href=$PREVU>$PREVT</a>+" >tmp.$$
  fi
  mv tmp.$$ $THISU
  PREVT=$THIST
  PREVU=$THISU
  THIST=$NEXTT
  THISU=$NEXTU
  echo adding previous/next links in $THISU ...
done

sed "s+NEXTPAGE+<a href=install.htm>Installing the WFDB Software Package</a>+" <$THISU | \
  sed "s+PREVPAGE+<a href=$PREVU>$PREVT</a>+" >tmp.$$
mv tmp.$$ $THISU

PREVT=$THIST
PREVU=$THISU
THIST="Installing the WFDB Software Package"
THISU=install.htm
NEXTT="Evaluating ECG Analyzers"
NEXTU=eval.htm

echo adding previous/next links in $THISU ...
sed "s+<IMG\\([^<>]*\\)SRC=\"prev_g.png\">+<a href=$PREVU><IMG\\1SRC=\"previous.png\"></a>+" <$THISU |
 sed "s+<IMG\\([^<>]*\\)SRC=\"next_g.png\">+<a href=$NEXTU><IMG\\1SRC=\"next.png\"></a>+" |
 sed "s+<B>Up:</B> <A+<b>Next:</b> <a href=$NEXTU>$NEXTT</a> <B>Up:</B> <A+" |
 sed "s+WFDB Applications Guide</A>+WFDB Applications Guide</A> <b>Previous:</b> <a href=$PREVU>$PREVT</a>+" >tmp.$$
mv tmp.$$ $THISU

PREVT=$THIST
PREVU=$THISU
THIST=$NEXTT
THISU=$NEXTU

echo adding previous links in $THISU ...
sed "s+<IMG\\([^<>]*\\)SRC=\"prev_g.png\">+<a href=$PREVU><IMG\\1SRC=\"previous.png\"></a>+" <$THISU |
 sed "s+WFDB Applications Guide</A>+WFDB Applications Guide</A> <b>Previous:</b> <a href=$PREVU>$PREVT</a>+" >tmp.$$
mv tmp.$$ $THISU
