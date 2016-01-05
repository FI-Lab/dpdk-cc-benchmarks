#!/bin/bash

TRACENO=../../abc_search/realrun/noloc
TRACELO=../../abc_search/realrun/loloc
TRACEHI=../../abc_search/realrun/hiloc
FIB=h1fib_stack_op

for i in `ls rules/`
do
    if [[ "$i" =~ "100K" ]]
    then
        echo "skip" $i
        continue
    fi

    if [[ "$i" =~ "10K" ]]
    then
        if [[ "$i" =~ "fw" ]]
        then
            echo "skip" $i
            continue
        fi
    fi


    ./realpc -r rules/$i -l $FIB/fib$i -t $TRACENO/${i}_trace 

done
