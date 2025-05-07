#!/bin/bash

g++ -fPIC -shared -o libparallel.so parallel_for.cpp -lpthread -std=c++17 -O3
# 编译测试程序
g++ -o heated_plate_pthreads heated_plate_pthreads.cpp -L. -lparallel -lpthread -lm

export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
./heated_plate