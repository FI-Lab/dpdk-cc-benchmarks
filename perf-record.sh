#!/bin/bash
perf record -e LLC-load-misses -g timeout --signal=SIGUSR1 30s ./build/fwd -c 3 -n 4 -- -p 2 -r data/pc/acl2_2_0.5_-0.1_10K -l data/pc/acl210K.fib -P data/dpi/snort-key -H data/hs/acl210K.hash 

perf report
