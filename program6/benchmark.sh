#!/bin/bash

# 生成 CSV 文件头
rm -f results.csv
echo "Program,Threads,Schedule,Time" > results.csv

# 线程配置和调度方式
THREADS=(1 2 4 8 16)
SCHEDULES=("static" "dynamic" "guided")

# Pthreads 版本测试
for threads in "${THREADS[@]}"; do
    for sched in "${SCHEDULES[@]}"; do
        echo "Running Pthreads: threads=$threads, schedule=$sched"
        TIME=$(./heated_plate_pthreads $threads $sched | grep "Time elapsed" | awk '{print $3}')
        echo "pthreads,$threads,$sched,$TIME" >> results.csv
    done
done

# OpenMP 版本测试（需要编译为 heated_plate_openmp）
for threads in "${THREADS[@]}"; do
    for sched in "${SCHEDULES[@]}"; do
        echo "Running OpenMP: threads=$threads, schedule=$sched"
        export OMP_NUM_THREADS=$threads
        export OMP_SCHEDULE=$sched
        TIME=$(./heated_plate_openmp | grep "Wallclock time" | awk '{print $4}')
        echo "openmp,$threads,$sched,$TIME" >> results.csv
    done
done
