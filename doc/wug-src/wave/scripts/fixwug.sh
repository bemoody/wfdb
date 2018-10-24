#!/bin/sh
# file: fixwug.sh		G. Moody	24 June 2002
#				Last revised:	24 October 2018

set -e

cd $1
rm -f wave-faq.htm wave-web.htm
R=`grep -l "<TITLE>Frequently Asked Questions</TITLE>" *.htm`
if [ "x$R" != "x" ]; then ln -s $R wave-faq.htm
else echo "Can't find wave-faq.htm"
fi
R=`grep -l "<TITLE>WAVE and the Web</TITLE>" *.htm`
if [ "x$R" != "x" ]; then ln -s $R wave-web.htm
else echo "Can't find wave-web.htm"
fi
