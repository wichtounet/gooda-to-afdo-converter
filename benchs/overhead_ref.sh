#!/bin/bash

sudo su
cd /home/wichtounet/gcc/google/spec/
source shrc
nohup make overhead_ref > overhead_log_1 &
