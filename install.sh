#!/bin/sh
# file: install.sh		G. Moody	7 June 2000

case $# in
 0|1) echo "usage: $0 DIRECTORY FILE [FILE ...]"
      exit ;;
esac

DIR=$1
if [ ! -e $DIR ]
then
    mkdir -p $DIR
fi

if [ ! -d $DIR ]
then
    echo "Can't install in $DIR -- exiting."
    exit
fi

shift

for FILE in $*
do
    if [ -e $FILE.exe ]
    then
        cp -p $FILE.exe $DIR
    else
        if [ -e $FILE ]
        then
	    cp -p $FILE $DIR
        else
	    echo "$FILE does not exist"
        fi
    fi
done
