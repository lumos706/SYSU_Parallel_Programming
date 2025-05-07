#!/bin/bash
g++ -fPIC -shared -o libparallel++.so parallel_for.cpp -lpthread -std=c++17
# 编译测试程序
g++ -o matmul matmul.cpp -L. -lparallel++ -lpthread -std=c++17

# 设置库路径并运行
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
./matmul