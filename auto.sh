#!/bin/bash

if [ $# != 1  ]; then
    exit 1
fi

table=("1" "3" "f" "3f" "303f" "3f03f")

ls log/$1 || mkdir log/$1

for ((i = 0; i < 6; i = i + 1));
do
    echo "$1-$i"
    ./setenv.sh 131071 512 ${table[$i]} &> log/$1/"$1$i"
done
