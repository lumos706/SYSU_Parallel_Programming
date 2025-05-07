#include <iostream>
#include <cmath>
#include <cstring>
#include <pthread.h>
#include <chrono>
#include "parallel_for.hpp"

#define M 500
#define N 500

double w[M][N];
double u[M][N];
double epsilon = 0.001;

struct UpdateArgs {
    double (*u)[N];
    double (*w)[N];
};

void copy_rows(int i, void* args) {
    double (*src)[N] = static_cast<double (*)[N]>(args);
    for (int j = 0; j < N; ++j)
        u[i][j] = src[i][j];
}

void update_rows(int i, void* args) {
    UpdateArgs* a = static_cast<UpdateArgs*>(args);
    for (int j = 1; j < N - 1; ++j)
        a->w[i][j] = (a->u[i-1][j] + a->u[i+1][j] + a->u[i][j-1] + a->u[i][j+1]) / 4.0;
}

struct DiffArgs {
    double* diff;
};

void compute_diff(int i, void* args) {
    DiffArgs* dargs = static_cast<DiffArgs*>(args);
    double local_diff = 0.0;
    for (int j = 1; j < N - 1; ++j) {
        double delta = std::fabs(w[i][j] - u[i][j]);
        if (delta > local_diff)
            local_diff = delta;
    }

#pragma omp critical
    {
        if (local_diff > *(dargs->diff))
            *(dargs->diff) = local_diff;
    }
}

int main(int argc, char* argv[]) {
    int num_threads = 4;
    SchedulePolicy policy = SchedulePolicy::Static;

    if (argc >= 2) num_threads = std::atoi(argv[1]);
    if (argc >= 3) {
        std::string sched = argv[2];
        if (sched == "dynamic") policy = SchedulePolicy::Dynamic;
        else if (sched == "guided") policy = SchedulePolicy::Guided;
    }

    std::cout << "\nHEATED_PLATE_PTHREADS\n";
    std::cout << "  Grid size: " << M << "x" << N << "\n";
    std::cout << "  Convergence threshold: " << epsilon << "\n";
    std::cout << "  Threads: " << num_threads << "\n";
    std::cout << "  Schedule: " << (policy == SchedulePolicy::Static ? "Static" :
                                      policy == SchedulePolicy::Dynamic ? "Dynamic" : "Guided") << "\n";

    // 设置边界
    double mean = 0.0;
    for (int i = 1; i < M - 1; ++i) {
        w[i][0] = 100.0;
        w[i][N-1] = 100.0;
        mean += w[i][0] + w[i][N-1];
    }
    for (int j = 0; j < N; ++j) {
        w[M-1][j] = 100.0;
        w[0][j] = 0.0;
        mean += w[M-1][j] + w[0][j];
    }

    mean /= (2 * M + 2 * N - 4);

    // 初始化内部点
    for (int i = 1; i < M - 1; ++i)
        for (int j = 1; j < N - 1; ++j)
            w[i][j] = mean;

    int iterations = 0;
    int iterations_print = 1;
    double diff = epsilon;

    std::cout << "\n Iteration  Change\n\n";
    auto start_time = std::chrono::high_resolution_clock::now();

    while (epsilon <= diff) {
        // 拷贝 w 到 u
        parallel_for(0, M, 1, copy_rows, w, num_threads, policy);

        // 计算新的 w 值
        UpdateArgs uargs = { u, w };
        parallel_for(1, M - 1, 1, update_rows, &uargs, num_threads, policy);

        // 比较最大差异
        double max_diff = 0.0;
        DiffArgs dargs = { &max_diff };
        parallel_for(1, M - 1, 1, compute_diff, &dargs, num_threads, policy);

        diff = max_diff;
        iterations++;
        if (iterations == iterations_print) {
            std::cout << "  " << iterations << "  " << diff << "\n";
            iterations_print *= 2;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "\n  " << iterations << "  " << diff << "\n";
    std::cout << "\n  Converged.\n";
    std::cout << "  Time elapsed: " << elapsed.count() << " seconds\n";
    std::cout << "HEATED_PLATE_PTHREADS: Normal end.\n";

    return 0;
}
