#! /bin/bash
#
gcc -c -Wall -fopenmp heated_plate_openmp.c
gcc -o heated_plate_openmp heated_plate_openmp.o -fopenmp
./heated_plate_openmp

