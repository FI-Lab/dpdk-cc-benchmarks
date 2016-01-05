#!/bin/bash

shopt -s compat31
m=h2


for i in `cat orderlist`
do
    if [[ "$i" =~ "100K" ]]
    then
        continue
    fi

    if [[ "$i" =~ "10K" ]]
    then
        if [[ "$i" =~ "fw" ]]
        then
            #echo "skip" $i
            continue
        fi
    fi


    #line=`grep "average memory access num: " "tr/${m}_${i}" `
    #echo $line
    line=`grep "max memory access: " "tr/${m}_${i}" `

    #[[ "$line" =~ "average memory access num: (.*)" ]]
    [[ "$line" =~ "max memory access: (.*)" ]]
    a=${BASH_REMATCH[1]}
    #echo $i
    echo $a

done


