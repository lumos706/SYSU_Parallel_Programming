#include <iostream>
#include <random>
#include <chrono>

int main() {
    int m = 512, n = 1024, k = 2048;
    std::cout << "Matrix A: " << m << "*" << n << std::endl;
    std::cout << "Matrix B: " << n << "*" << k << std::endl;
    std::cout << "Matrix C: " << m << "*" << k << std::endl;
    double *A = new double[m * n];
    double *B = new double[n * k];
    double *C = new double[m * k]();

    // 随机数生成器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    // 填充A和B
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            A[i * n + j] = dis(gen);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < k; j++)
            B[i * k + j] = dis(gen);

    auto start = std::chrono::high_resolution_clock::now();

    // 三重循环矩阵乘法（i-j-p顺序）
    for (int i = 0; i < m; i++)
        for (int j = 0; j < k; j++)
            for (int p = 0; p < n; p++)
                C[i * k + j] += A[i * n + p] * B[p * k + j];

    auto end = std::chrono::high_resolution_clock::now();
    double time = std::chrono::duration<double>(end - start).count();
    std::cout << "运行时间: " << time << " s" << std::endl;
    double gflops = 2.0 * m * n * k / time / 1e9;
    std::cout << "GFLOPS: " << gflops << std::endl;
    double peak_gflops = 112.8;
    std::cout << "峰值性能百分比: " << gflops / peak_gflops * 100.0 << "%" << std::endl;
    delete[] A;
    delete[] B;
    delete[] C;
    return 0;
}