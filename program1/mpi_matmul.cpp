// mpic++ -o mpi_matmul mpi_matmul.cpp -O3 -std=c++17
// mpirun --oversubscribe -np 1 ./mpi_matmul 128 128 128
#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>

using namespace std;
//输入参数：num_procs, m, n, k
int main(int argc, char** argv) {
    // 初始化MPI环境
    MPI_Init(&argc, &argv);
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int m, n, k;
    if (rank == 0) {
        // 检测参数个数是否正确
        if (argc != 4) {
            cerr << "Usage: " << argv[0] << " m n k" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        m = atoi(argv[1]);
        n = atoi(argv[2]);
        k = atoi(argv[3]);
        if (m < 128 || m > 2048 || n < 128 || n > 2048 || k < 128 || k > 2048) {
            cerr << "m, n, k must be in [128, 2048]" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        // 检查m是否能被进程数整除（否则分块不均匀）
        if (m % num_procs != 0) {
            cerr << "m must be divisible by number of processes" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);

    double *A = nullptr, *B = nullptr, *C = nullptr;
    // 主进程负责生成初始矩阵A和B
    if (rank == 0) {
        A = new double[m * n];
        B = new double[n * k];
        C = new double[m * k];
        srand(time(nullptr));
        for (int i = 0; i < m * n; ++i) A[i] = (double)rand() / RAND_MAX;
        for (int i = 0; i < n * k; ++i) B[i] = (double)rand() / RAND_MAX;
    }
    // 分配本地矩阵B
    int rows_per_process = m / num_procs;
    double *B_local = new double[n * k];
    
    // 分配本地矩阵A
    if (rank == 0) {
        for (int dest = 1; dest < num_procs; ++dest) {
            MPI_Send(B, n * k, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
        }
        memcpy(B_local, B, n * k * sizeof(double));
    } else {
        MPI_Recv(B_local, n * k, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    double *A_local = new double[rows_per_process * n];
    if (rank == 0) {
        for (int dest = 1; dest < num_procs; ++dest) {
            int start_row = dest * rows_per_process;
            MPI_Send(A + start_row * n, rows_per_process * n, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
        }
        memcpy(A_local, A, rows_per_process * n * sizeof(double));
    } else {
        MPI_Recv(A_local, rows_per_process * n, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // 开始计时
    double start_time = MPI_Wtime();
    
    // 最简单的一集
    double *C_local = new double[rows_per_process * k];
    for (int i = 0; i < rows_per_process; ++i) {
        for (int j = 0; j < k; ++j) {
            C_local[i * k + j] = 0.0;
            for (int l = 0; l < n; ++l) {
                C_local[i * k + j] += A_local[i * n + l] * B_local[l * k + j];
            }
        }
    }

    if (rank == 0) {
        memcpy(C, C_local, rows_per_process * k * sizeof(double));
        for (int src = 1; src < num_procs; ++src) {
            MPI_Recv(C + src * rows_per_process * k, rows_per_process * k, 
                     MPI_DOUBLE, src, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    } else {
        MPI_Send(C_local, rows_per_process * k, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        double end_time = MPI_Wtime();
        double duration = end_time - start_time;

        cout << "Time elapsed: " << duration << " seconds" << endl;

        delete[] A;
        delete[] B;
        delete[] C;
    }

    delete[] A_local;
    delete[] B_local;
    delete[] C_local;

    MPI_Finalize();
    return 0;
}