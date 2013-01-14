#!/bin/bash

sudo su
cd /home/wichtounet/gcc/google/spec/
source shrc
nohup make bench_ref > bench_log_1 &
