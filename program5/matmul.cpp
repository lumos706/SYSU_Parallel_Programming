#include "parallel_for.hpp"
#include <vector>
#include <chrono>
#include <iostream>

struct MatrixArgs {
    std::vector<float>& A;
    std::vector<float>& B;
    std::vector<float>& C;
    int n, k;
};

void matrix_multiply(int i, void* args) {
    MatrixArgs* data = static_cast<MatrixArgs*>(args);
    for(int j = 0; j < data->k; ++j) {
        float sum = 0.0f;
        for(int l = 0; l < data->n; ++l) {
            sum += data->A[i * data->n + l] * data->B[l * data->k + j];
        }
        data->C[i * data->k + j] = sum;
    }
}

int main() {
    const int N = 2048;
    std::vector<float> A(N*N, 1.0f), B(N*N, 1.0f), C(N*N, 0.0f);
    MatrixArgs args{A, B, C, N, N};

    auto start = std::chrono::high_resolution_clock::now();
    
    parallel_for(0, N, 1, 
                matrix_multiply, &args,
                16, SchedulePolicy::Dynamic); // 使用动态调度

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time: " << elapsed.count() << "s\n";

    return 0;
}