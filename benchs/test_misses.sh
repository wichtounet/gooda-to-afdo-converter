#!/bin/bash

export GOODA_DIR=/home/users/atlas/gooda/gooda-analyzer/
../gcc-afdo-misses-install/bin/g++ -g -O2 bench_read_read.cpp
../build/bin/converter --addr2line="/home/wichtounet/gcc/google/binutils/binutils_install/bin/addr2line" --nows --cache-misses --quiet --bench --profile --afdo ./a.out
../gcc-afdo-misses-install/bin/g++ -g -O2 -fauto-profile bench_read_read.cpp

../build/bin/converter --nows --cache-misses --read-afdo --full-dump fbdata.afdo
