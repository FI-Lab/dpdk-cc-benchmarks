#!/bin/bash

TRACENO=../../abc_search/realrun/noloc
TRACELO=../../abc_search/realrun/loloc
TRACEHI=../../abc_search/realrun/hiloc
FIB=ophifibh2
#m=${FIB%fib}
m=hih2
echo $m
REPORT=hireport

for i in `ls rules/`
do
    if [[ "$i" =~ "100K" ]]
    then
        echo "skip" $i
        continue
    fi

    #if [[ "$i" =~ "10K" ]]
    #then
    #    if [[ "$i" =~ "fw" ]]
    #    then
    #        echo "skip" $i
    #        continue
    #    fi
    #fi


    for (( j=0; $j<10; $((j++)) ))
    do
        echo "no loc" $i
        ./realpc -r rules/$i -l $FIB/fib$i -t $TRACENO/${i}_trace >> $REPORT/noloc_${m}_$i
    done

    for (( j=0; $j<10; $((j++)) ))
    do
        echo "lo loc" $i
        ./realpc -r rules/$i -l $FIB/fib$i -t $TRACELO/${i}_trace >> $REPORT/loloc_${m}_$i
    done

    for (( j=0; $j<10; $((j++)) ))
    do
        echo "hi loc" $i
        ./realpc -r rules/$i -l $FIB/fib$i -t $TRACEHI/${i}_trace >> $REPORT/hiloc_${m}_$i
    done
done
