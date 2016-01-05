#!/bin/bash

for i in `ls rules`
do
    if [[ "$i" =~ "100K" ]]
    then
        continue
    fi

    while read -r line
    do
        echo $line
        #"$j" =~ "ave time: (.*)"
        #echo ${BASH_REMACH[1]}
    done << `grep "ave time: " "report/noloc_${i}" `
done
