#!/bin/bash
FIB=h1fib_stack_op
PARA=-h1

for i in `ls rules`
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

    echo "building" $i
    ../hypc -r rules/$i $PARA -o $FIB/fib$i
done
