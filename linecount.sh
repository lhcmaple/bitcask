#!/bin/bash

function countline {
    totalline=0
    for name in `ls`
    do
        if [ -d $name ] 
        then
            cd $name
            line=`countline $name`
            totalline=$((totalline + line))
            cd ..
        fi
    done
    for name in `ls | grep -E "(.cc$)|(\.h$)" | grep -v "test\.cc"`
    do
        line=`wc -l $name | awk '{print $1}'`
        wc -l $name >&2
        totalline=$((totalline + line))
    done
    echo $totalline
}

echo "-----total lines of bitcask: `countline` lines-----"
