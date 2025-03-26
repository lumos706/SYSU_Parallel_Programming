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
    double *C = new double[m * k](); // 初始化为0

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

    // 列分块展开（j-i-p顺序，保持原逻辑）
    int j, i, p;
    for (j = 0; j < (k & (~3)); j += 4) { // 每次处理4列
        for (i = 0; i < m; i++) {        // 遍历A的行
            // 预加载B的4个元素（同一行p的连续4列）
            double b0 = B[i * k + j];
            double b1 = B[i * k + j + 1];
            double b2 = B[i * k + j + 2];
            double b3 = B[i * k + j + 3];
            // 遍历C的列（即B的列）
            for (p = 0; p < n; p++) {
                double a = A[i * n + p]; // A(i,p)
                // 关键修正：C的行由i决定，列由j+col决定
                C[i * k + j]     += a * b0;
                C[i * k + j + 1] += a * b1;
                C[i * k + j + 2] += a * b2;
                C[i * k + j + 3] += a * b3;
            }
        }
    }
    // 处理剩余列（当k不是4的倍数时）
    for (; j < k; j++) {
        for (i = 0; i < m; i++) {
            double b0 = B[i * k + j];
            for (p = 0; p < n; p++) {
                C[i * k + j] += A[i * n + p] * b0;
            }
        }
    }

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