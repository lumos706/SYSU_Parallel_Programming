#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>

// 并行矩阵乘法
struct MatrixArgs {
    int start_row;
    int end_row;
    int n;
    std::vector<std::vector<double>>* A;
    std::vector<std::vector<double>>* B;
    std::vector<std::vector<double>>* C;
};

void matrix_mul(MatrixArgs args) {
    for (int i = args.start_row; i < args.end_row; i++) {
        for (int j = 0; j < args.n; j++) {
            double sum = 0.0;
            for (int k = 0; k < args.n; k++) {
                sum += (*args.A)[i][k] * (*args.B)[k][j];
            }
            (*args.C)[i][j] = sum;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <matrix_size> <thread_num>\n";
        return 1;
    }

    int n = std::stoi(argv[1]);
    int num_threads = std::stoi(argv[2]);

    // 初始化矩阵
    std::vector<std::vector<double>> A(n, std::vector<double>(n));
    std::vector<std::vector<double>> B(n, std::vector<double>(n));
    std::vector<std::vector<double>> C(n, std::vector<double>(n, 0.0));

    // 填充矩阵A和B
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A[i][j] = dis(gen);
            B[i][j] = dis(gen);
        }
    }

    // 创建线程
    std::vector<std::thread> threads;
    std::vector<MatrixArgs> args(num_threads);
    int rows_per_thread = n / num_threads;

    auto start = std::chrono::high_resolution_clock::now();

    for (int t = 0; t < num_threads; t++) {
        args[t].start_row = t * rows_per_thread;
        args[t].end_row = (t == num_threads - 1) ? n : (t + 1) * rows_per_thread;
        args[t].n = n;
        args[t].A = &A;
        args[t].B = &B;
        args[t].C = &C;
        threads.emplace_back(matrix_mul, args[t]);
    }

    // 等待线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_taken = end - start;
    std::cout << n << " " << num_threads << " Time: " << time_taken.count() << " seconds\n";

    return 0;
}