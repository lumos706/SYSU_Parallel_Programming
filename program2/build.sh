mpic++ -o mpi_matmul_v2 mpi_matmul_v2.cpp -O3 -std=c++17
mpirun --oversubscribe -np 1 ./mpi_matmul_v2 128 128 128
mpirun --oversubscribe -np 2 ./mpi_matmul_v2 128 128 128
mpirun --oversubscribe -np 4 ./mpi_matmul_v2 128 128 128
mpirun --oversubscribe -np 8 ./mpi_matmul_v2 128 128 128
mpirun --oversubscribe -np 16 ./mpi_matmul_v2 128 128 128
mpirun --oversubscribe -np 1 ./mpi_matmul_v2 256 256 256
mpirun --oversubscribe -np 2 ./mpi_matmul_v2 256 256 256
mpirun --oversubscribe -np 4 ./mpi_matmul_v2 256 256 256
mpirun --oversubscribe -np 8 ./mpi_matmul_v2 256 256 256
mpirun --oversubscribe -np 16 ./mpi_matmul_v2 256 256 256
mpirun --oversubscribe -np 1 ./mpi_matmul_v2 512 512 512
mpirun --oversubscribe -np 2 ./mpi_matmul_v2 512 512 512
mpirun --oversubscribe -np 4 ./mpi_matmul_v2 512 512 512
mpirun --oversubscribe -np 8 ./mpi_matmul_v2 512 512 512
mpirun --oversubscribe -np 16 ./mpi_matmul_v2 512 512 512
mpirun --oversubscribe -np 1 ./mpi_matmul_v2 1024 1024 1024
mpirun --oversubscribe -np 2 ./mpi_matmul_v2 1024 1024 1024
mpirun --oversubscribe -np 4 ./mpi_matmul_v2 1024 1024 1024
mpirun --oversubscribe -np 8 ./mpi_matmul_v2 1024 1024 1024
mpirun --oversubscribe -np 16 ./mpi_matmul_v2 1024 1024 1024
mpirun --oversubscribe -np 1 ./mpi_matmul_v2 2048 2048 2048
mpirun --oversubscribe -np 2 ./mpi_matmul_v2 2048 2048 2048
mpirun --oversubscribe -np 4 ./mpi_matmul_v2 2048 2048 2048
mpirun --oversubscribe -np 8 ./mpi_matmul_v2 2048 2048 2048
mpirun --oversubscribe -np 16 ./mpi_matmul_v2 2048 2048 2048