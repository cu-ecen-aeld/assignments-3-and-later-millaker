#!/bin/sh

# Check number of arguments
if [ $# -ne 2 ]
then
    echo "Not enough arguments, got $#"
    echo "Usage: writer.sh FILENAME STRING"
    exit 1
else
    FILEPATH=$(dirname $1)
    FILENAME=$(basename $1)
    STR=$2
fi

#echo $FILEPATH
#echo $FILENAME
#echo $FILEPATH/$FILENAME
#echo $STR

FULL=$FILEPATH/$FILENAME

# Check dir exist
mkdir -p $FILEPATH
touch $FULL
echo $STR > $FULL
