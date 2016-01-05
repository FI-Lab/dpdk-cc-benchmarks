#!/bin/bash


shopt -s compat31
IFS=$'\n'
#!!!!!!!!!
m=hih2
REPORT=hireport

for i in `cat orderlist`
do
    if [[ "$i" =~ "100K" ]]
    then
        continue
    fi

    #if [[ "$i" =~ "10K" ]]
    #then
    #    if [[ "$i" =~ "fw" ]]
    #    then
    #        #echo "skip" $i
    #        continue
    #    fi
    #fi


    grep "ave time" "$REPORT/noloc_${m}_${i}" > temp 
    #mv "report/hiloc_${i}" "report/hiloc_${m}_${i}"
    sum=0

    while read -r line
    do
        #echo "$line"
        [[ "$line" =~ "ave time: (.*)ns" ]]
        a=${BASH_REMATCH[1]}
        sum=` perl -e "print $a+$sum" `
        #echo "sum" $sum
    done < "temp" 

    avg=` perl -e "print $sum/10" `
    #echo $i
    echo $avg

done

rm temp

