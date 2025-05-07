#include <pthread.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <chrono>
#include <random>

using namespace std;

// ================== 一元二次方程求解（并行） ==================
struct EquationArgs {
    double a, b, c;
    double delta;
    double denominator;
    bool delta_ready;
    bool denominator_ready;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

// 计算判别式
void* calc_delta(void* arg) {
    EquationArgs* args = (EquationArgs*)arg;
    double a = args->a, b = args->b, c = args->c;
    
    pthread_mutex_lock(&args->mutex);
    args->delta = b * b - 4 * a * c;
    args->delta_ready = true;
    pthread_cond_signal(&args->cond);
    pthread_mutex_unlock(&args->mutex);
    return nullptr;
}

// 计算分母
void* calc_denominator(void* arg) {
    EquationArgs* args = (EquationArgs*)arg;
    double a = args->a;
    
    pthread_mutex_lock(&args->mutex);
    args->denominator = 2 * a;
    args->denominator_ready = true;
    pthread_cond_signal(&args->cond);
    pthread_mutex_unlock(&args->mutex);
    return nullptr;
}

void solve_quadratic_parallel(double a, double b, double c) {
    EquationArgs args = {a, b, c, 0, 0, false, false, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER};
    
    pthread_t t1, t2;
    pthread_create(&t1, nullptr, calc_delta, &args);
    pthread_create(&t2, nullptr, calc_denominator, &args);

    // 等待两个线程完成
    pthread_mutex_lock(&args.mutex);
    while (!(args.delta_ready && args.denominator_ready)) {
        pthread_cond_wait(&args.cond, &args.mutex);
    }
    pthread_mutex_unlock(&args.mutex);

    // 计算结果
    if (args.denominator == 0) {
        // 非二次方程
    } else if (args.delta < 0) {
        // 复数根
    } else {
        double sqrt_delta = sqrt(args.delta);
        double x1 = (-b + sqrt_delta) / args.denominator;
        double x2 = (-b - sqrt_delta) / args.denominator;
    }

    pthread_mutex_destroy(&args.mutex);
    pthread_cond_destroy(&args.cond);
}

// ================== 一元二次方程求解（串行） ==================
void solve_quadratic_serial(double a, double b, double c) {
    double delta = b * b - 4 * a * c;
    double denominator = 2 * a;

    if (denominator == 0) {
        // 非二次方程
    } else if (delta < 0) {
        // 复数根
    } else {
        double sqrt_delta = sqrt(delta);
        double x1 = (-b + sqrt_delta) / denominator;
        double x2 = (-b - sqrt_delta) / denominator;
    }
}

// ================== 主函数 ==================
int main() {
    const int iterations = 100; // 运行次数
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(-100.0, 100.0);

    // 并行求解
    auto start_parallel = chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        double a = dis(gen);
        double b = dis(gen);
        double c = dis(gen);
        solve_quadratic_parallel(a, b, c);
    }
    auto end_parallel = chrono::high_resolution_clock::now();
    chrono::duration<double> parallel_time = end_parallel - start_parallel;

    // 串行求解
    auto start_serial = chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        double a = dis(gen);
        double b = dis(gen);
        double c = dis(gen);
        solve_quadratic_serial(a, b, c);
    }
    auto end_serial = chrono::high_resolution_clock::now();
    chrono::duration<double> serial_time = end_serial - start_serial;

    // 输出结果
    cout << "并行求解平均时间: " << (parallel_time.count() / iterations)*1000.0 << " 毫秒" << endl;
    cout << "串行求解平均时间: " << (serial_time.count() / iterations)*1000.0 << " 毫秒" << endl;

    return 0;
}