mpic++ -O3 -o sparse_matmul sparse_matmul.cpp
mpirun --oversubscribe -np 16 ./sparse_matmul 2048 2048 2048