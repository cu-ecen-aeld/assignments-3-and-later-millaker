#!/bin/sh

# Check number of arguments
if [ $# -ne 2 ]
then
    echo "Not enough arguments, got $#"
    echo "Usage: finder.sh PATH STRING"
    exit 1
else
    FINDPATH=$1
    FINDSTR=$2
fi

# Check if given path is a directory
if [ ! -d $FINDPATH ]
then
    echo "$FINDPATH does not represent a directory"
    exit 1
fi

file_num=0
matching_lines=0

# Search for matching string recursively
search_dir(){
    for entry in $1/*
    do
        # Chech if is directory
        if [ -d $entry ]
        then
            # echo "Entering $entry"
            search_dir $entry
        else
            # Count files
            file_num=$(( file_num + 1 ))
            # search for string matches
            temp="$(grep $FINDSTR $entry -c)"
            matching_lines=$(( matching_lines + temp ))
        fi
    done
}

search_dir $FINDPATH
echo "The number of files are $file_num and the number of matching lines are $matching_lines"
