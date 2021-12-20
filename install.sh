#!/bin/sh
# file: install.sh		G. Moody	7 June 2000
#                 		Last revised:	20 December 2021

set -e

case $# in
 0|1) echo "usage: $0 DIRECTORY FILE [FILE ...]"
      exit 1 ;;
esac

DIR=$1
if [ ! -d "$DIR" ]
then
    mkdir -p "$DIR"
fi

if [ ! -d "$DIR" ]
then
    echo "Can't install in $DIR -- exiting."
    exit 1
fi

shift

for FILE in "$@"
do
    if [ -f "$FILE.exe" ]
    then
        cp -p "$FILE.exe" "$DIR"
    else
        if [ -f "$FILE" ]
        then
	    cp -p "$FILE" "$DIR"
        else
	    echo "$FILE does not exist"
	    exit 1
        fi
    fi
done
