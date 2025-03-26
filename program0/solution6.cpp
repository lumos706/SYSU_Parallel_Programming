//g++ -o solution6 solution6.cpp -lmkl_rt -liomp5 -L/opt/intel/oneapi/compiler/2025.0/lib -I/opt/intel/oneapi/mkl/latest/include -Wl,-rpath=/opt/intel/oneapi/compiler/2025.0/lib &&./solution6
#include <mkl.h>
#include <iostream>
#include <chrono>
#include <random>

int main() {
    const int m = 512, n = 1024, k = 2048;
    std::cout << "Matrix A: " << m << "*" << n << std::endl;
    std::cout << "Matrix B: " << n << "*" << k << std::endl;
    std::cout << "Matrix C: " << m << "*" << k << std::endl;
    const double alpha = 1.0, beta = 0.0;
    const double PEAK_GFLOPS = 112.8; 

    // 使用mkl_malloc分配64字节对齐内存
    double *A = (double *)mkl_malloc(m * k * sizeof(double), 64);
    double *B = (double *)mkl_malloc(k * n * sizeof(double), 64);
    double *C = (double *)mkl_malloc(m * n * sizeof(double), 64);

    if (A == nullptr || B == nullptr || C == nullptr) {
        std::cerr << "内存分配失败！" << std::endl;
        mkl_free(A); mkl_free(B); mkl_free(C);
        return 1;
    }

    // 使用随机数填充矩阵（更符合实验要求）
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    for (int i = 0; i < m * k; i++) A[i] = dis(gen);
    for (int i = 0; i < k * n; i++) B[i] = dis(gen);
    for (int i = 0; i < m * n; i++) C[i] = 0.0;

    // 精确计时（仅计算dgemm时间）
    auto start = std::chrono::high_resolution_clock::now();
    cblas_dgemm(
        CblasRowMajor, CblasNoTrans, CblasNoTrans,
        m, n, k, alpha, A, k, B, n, beta, C, n
    );
    auto end = std::chrono::high_resolution_clock::now();

    // 性能指标计算
    double time_sec = std::chrono::duration<double>(end - start).count();
    double total_flops = 2.0 * m * n * k; // dgemm浮点操作数公式：2*m*n*k
    double gflops = (total_flops / time_sec) / 1e9;
    double peak_percent = (gflops / PEAK_GFLOPS) * 100;

    // 输出结果
    std::cout << "运行时间: " << time_sec << " s" << std::endl;
    std::cout << "GFLOPS: " << gflops << std::endl;
    std::cout << "峰值性能百分比: " << peak_percent << "%" << std::endl;

    // 释放内存
    mkl_free(A); mkl_free(B); mkl_free(C);
    return 0;
}