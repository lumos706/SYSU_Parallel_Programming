#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>

using namespace std;

typedef struct {
    int m;
    int n;
    int k;
} MatrixDims;

void create_matrix_dim_type(MPI_Datatype *mpi_matrix_dim) {
    const int nitems = 3;
    int blocklengths[3] = {1, 1, 1};
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint offsets[3];

    offsets[0] = offsetof(MatrixDims, m);
    offsets[1] = offsetof(MatrixDims, n);
    offsets[2] = offsetof(MatrixDims, k);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, mpi_matrix_dim);
    MPI_Type_commit(mpi_matrix_dim);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    MPI_Datatype MPI_MATRIX_DIM;
    create_matrix_dim_type(&MPI_MATRIX_DIM);

    MatrixDims dims;
    double *A = nullptr, *B = nullptr;
    double *A_local, *B_local, *C_local;
    int rows_per_process;

    // 第一阶段：参数处理与维度广播
    if (rank == 0) {
        if (argc != 4) {
            cerr << "Usage: " << argv[0] << " m n k" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        dims.m = atoi(argv[1]);
        dims.n = atoi(argv[2]);
        dims.k = atoi(argv[3]);

        if (dims.m % num_procs != 0) {
            cerr << "m must be divisible by number of processes" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    
    MPI_Bcast(&dims, 1, MPI_MATRIX_DIM, 0, MPI_COMM_WORLD);
    rows_per_process = dims.m / num_procs;

    // 统一为所有进程分配B_local内存
    B_local = new double[dims.n * dims.k];  // 所有进程统一分配

    // 主进程生成数据
    if (rank == 0) {
        A = new double[dims.m * dims.n];
        B = new double[dims.n * dims.k];
        
        srand(time(nullptr));
        for (int i = 0; i < dims.m * dims.n; ++i)
            A[i] = (double)rand() / RAND_MAX;
        for (int i = 0; i < dims.n * dims.k; ++i)
            B[i] = (double)rand() / RAND_MAX;
        
        memcpy(B_local, B, dims.n * dims.k * sizeof(double));
    }

    A_local = new double[rows_per_process * dims.n];
    MPI_Scatter(A, rows_per_process * dims.n, MPI_DOUBLE,
                A_local, rows_per_process * dims.n, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    MPI_Bcast(B_local, dims.n * dims.k, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    C_local = new double[rows_per_process * dims.k]{0}; // 初始化为0
    double start_time = MPI_Wtime();
    
    for (int i = 0; i < rows_per_process; ++i) {
        for (int j = 0; j < dims.k; ++j) {
            for (int l = 0; l < dims.n; ++l) {
                C_local[i * dims.k + j] += A_local[i * dims.n + l] * B_local[l * dims.k + j];
            }
        }
    }

    double* C = nullptr;
    if (rank == 0) {
        C = new double[dims.m * dims.k];
    }
    
    MPI_Gather(C_local, rows_per_process * dims.k, MPI_DOUBLE,
               C, rows_per_process * dims.k, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        double end_time = MPI_Wtime();
        cout << "Matrix -- " << argv[1] << " Time elapsed: " << (end_time - start_time) * 1000 << " ms" << endl;
        delete[] A;
        delete[] B;
        delete[] C;
    }

    delete[] A_local;
    delete[] B_local;
    delete[] C_local;
    MPI_Type_free(&MPI_MATRIX_DIM);
    MPI_Finalize();
    return 0;
}