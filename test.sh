#!/bin/bash
perf stat -e task-clock -e cpu-clock -e bus-cycles -e cs -e faults -e migrations -e stalled-cycles-frontend \
	  -e cycles -e instructions -e L1-dcache-load-misses -e L1-dcache-store-misses -e L1-dcache-prefetch-misses -e L1-icache-load-misses \
	  -e LLC-loads -e LLC-load-misses -e LLC-stores -e LLC-store-misses -e LLC-prefetches -e LLC-prefetch-misses \
      -e cache-references -e cache-misses timeout --signal=SIGUSR1 30s ./build/fwd -c 3f03f -n 4 -- -p 2 -r data/pc/acl2_2_0.5_-0.1_10K -l data/pc/acl210K.fib -P data/dpi/snort-key -H data/hs/acl210K.hash
