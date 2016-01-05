#!/bin/bash

sed -i s/"^#define NB_MBUF .*"/"#define NB_MBUF ${1}"/g main.c
sed -i s/"^#define NB_LCACHE_SIZE .*"/"#define NB_LCACHE_SIZE ${2}"/g main.c
sed -i s/"-c [0-9a-f]* "/"-c ${3} "/g test.sh

make clean;
make

rm ./paddr/*

./test.sh

wc paddr/*
