#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>

using namespace std;

struct SparseMatrixCSR {
    double* values;
    int* col_indices;
    int* row_ptr;
    int nnz;
    int rows;
    int cols;
};

void generate_sparse_matrix(SparseMatrixCSR &mat, int rows, int cols) {
    vector<double> values;
    vector<int> col_indices;
    vector<int> row_ptr(rows+1);
    
    srand(time(nullptr));
    int nnz = 0;
    for(int i=0; i<rows; ++i) {
        row_ptr[i] = nnz;
        for(int j=0; j<cols; ++j) {
            if(rand() % 20 == 0) { // 5%稀疏度
                values.push_back((double)rand()/RAND_MAX);
                col_indices.push_back(j);
                nnz++;
            }
        }
    }
    row_ptr[rows] = nnz;
    
    mat.rows = rows;
    mat.cols = cols;
    mat.nnz = nnz;
    mat.values = new double[nnz];
    mat.col_indices = new int[nnz];
    mat.row_ptr = new int[rows+1];
    
    copy(values.begin(), values.end(), mat.values);
    copy(col_indices.begin(), col_indices.end(), mat.col_indices);
    copy(row_ptr.begin(), row_ptr.end(), mat.row_ptr);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    SparseMatrixCSR A;
    int m, n, k;
    double *B_local = nullptr, *C_local = nullptr;

    // 参数处理与广播
    if(rank == 0) {
        if(argc != 4) {
            cerr << "Usage: " << argv[0] << " m n k" << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        m = atoi(argv[1]);
        n = atoi(argv[2]);
        k = atoi(argv[3]);
        
        generate_sparse_matrix(A, m, n);
    }

    // 关键修改1：先广播维度再分配内存
    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // 关键修改2：所有进程统一分配B_local
    B_local = new double[n*k];

    // 处理稀疏矩阵元数据
    if(rank == 0) {
        MPI_Bcast(A.row_ptr, m+1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&A.nnz, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(A.values, A.nnz, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(A.col_indices, A.nnz, MPI_INT, 0, MPI_COMM_WORLD);
        
        // 生成并广播密集矩阵B
        for(int i=0; i<n*k; ++i) 
            B_local[i] = (double)rand()/RAND_MAX;
    } else {
        A.row_ptr = new int[m+1];
        MPI_Bcast(A.row_ptr, m+1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&A.nnz, 1, MPI_INT, 0, MPI_COMM_WORLD);
        A.values = new double[A.nnz];
        A.col_indices = new int[A.nnz];
        MPI_Bcast(A.values, A.nnz, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(A.col_indices, A.nnz, MPI_INT, 0, MPI_COMM_WORLD);
    }

    // 关键修改3：统一广播B_local数据
    MPI_Bcast(B_local, n*k, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // 计算任务划分
    int rows_per_proc = m / num_procs;
    int start_row = rank * rows_per_proc;
    int end_row = (rank == num_procs-1) ? m : start_row + rows_per_proc;

    // 分配本地结果内存并初始化
    C_local = new double[(end_row - start_row) * k]{0};

    // 稀疏矩阵乘法核心
    double t_start = MPI_Wtime();
    for(int i=start_row; i<end_row; ++i) {
        for(int p=A.row_ptr[i]; p<A.row_ptr[i+1]; ++p) {
            int col = A.col_indices[p];
            double val = A.values[p];
            for(int j=0; j<k; ++j) {
                C_local[(i-start_row)*k + j] += val * B_local[col*k + j];
            }
        }
    }

    // 收集结果
    double* C = nullptr;
    if(rank == 0) C = new double[m*k];
    MPI_Gather(C_local, (end_row - start_row)*k, MPI_DOUBLE,
               C, rows_per_proc*k, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    // 处理剩余行（当m不能被进程数整除时）
    if(rank == 0 && m % num_procs != 0) {
        int remaining_start = num_procs * rows_per_proc;
        for(int i=remaining_start; i<m; ++i) {
            for(int p=A.row_ptr[i]; p<A.row_ptr[i+1]; ++p) {
                int col = A.col_indices[p];
                double val = A.values[p];
                for(int j=0; j<k; ++j) {
                    C[i*k + j] += val * B_local[col*k + j];
                }
            }
        }
    }

    // 输出结果
    if(rank == 0) {
        double t_end = MPI_Wtime();
        cout << "Time: " << (t_end - t_start)*1000 << " ms" << endl;
        delete[] C;
    }

    // 清理内存
    delete[] B_local;
    delete[] C_local;
    delete[] A.row_ptr;
    delete[] A.values;
    delete[] A.col_indices;

    MPI_Finalize();
    return 0;
}