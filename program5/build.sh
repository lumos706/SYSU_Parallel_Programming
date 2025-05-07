#!/bin/bash
g++ -fopenmp -O3 -o omp_matmul_cpp omp_matmul.cpp
# 删除旧的结果文件
rm -f results.csv

# 添加CSV表头
echo "Threads,m,n,k,Schedule,Time(s)" >> results.csv

# 定义测试参数
matrix_sizes=(128 256 512 1024 2048)
thread_counts=(1 2 4 8 16)
schedules=("static" "dynamic" "guided")

# 执行不同线程数和矩阵规模的测试
for threads in "${thread_counts[@]}"; do
  for size in "${matrix_sizes[@]}"; do
    OMP_NUM_THREADS=$threads ./omp_matmul_cpp $size $size $size >> results.csv
  done
done

# 测试不同调度方式
for schedule in "${schedules[@]}"; do
  OMP_NUM_THREADS=8 OMP_SCHEDULE=$schedule \
  ./omp_matmul_cpp 2048 2048 2048 >> results.csv
done

echo "所有测试已完成，结果保存在results.csv"