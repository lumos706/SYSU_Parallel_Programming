#include <pthread.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <chrono>
#include <random>

using namespace std;

// ================== 蒙特卡洛求π ==================
struct MonteCarloArgs {
    int total_points;
    int local_count;
    unsigned int seed;
};

void* monte_carlo(void* arg) {
    MonteCarloArgs* args = (MonteCarloArgs*)arg;
    int count = 0;

    // 使用独立的随机数生成器
    std::mt19937 gen(args->seed);
    std::uniform_real_distribution<> dis(-1.0, 1.0);

    for (int i = 0; i < args->total_points; ++i) {
        double x = dis(gen);
        double y = dis(gen);
        if (x * x + y * y <= 1) count++;
    }
    args->local_count = count;
    return nullptr;
}

void estimate_pi(int n, int num_threads) {
    auto start_pi = chrono::high_resolution_clock::now();

    vector<pthread_t> threads(num_threads);
    vector<MonteCarloArgs> args(num_threads);
    int points_per_thread = n / num_threads;

    // 初始化随机种子
    unsigned int main_seed = time(nullptr);

    // 创建线程
    for (int i = 0; i < num_threads; ++i) {
        args[i].total_points = points_per_thread;
        args[i].seed = main_seed + i;  // 不同线程使用不同种子
        pthread_create(&threads[i], nullptr, monte_carlo, &args[i]);
    }

    // 等待线程并汇总结果
    int total_hits = 0;
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
        total_hits += args[i].local_count;
    }

    // 处理剩余点数（当 n 不能被线程数整除时）
    int remaining = n % num_threads;
    if (remaining > 0) {
        MonteCarloArgs remain_arg;
        remain_arg.total_points = remaining;
        remain_arg.seed = main_seed + num_threads;
        monte_carlo(&remain_arg);
        total_hits += remain_arg.local_count;
    }

    auto end_pi = chrono::high_resolution_clock::now();
    double pi = 4.0 * total_hits / n;
    cout << "n=" << n << ", num_threads=" << num_threads << ", m=" << total_hits << ", π≈" << pi << "  ";
    chrono::duration<double> pi_time = end_pi - start_pi;
    cout << "蒙特卡洛求π时间: " << pi_time.count() * 1000.0 << " 毫秒" << endl;
}

// ================== 主函数 ==================
int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <total_points> <num_threads>" << endl;
        return 1;
    }

    // 解析命令行参数
    int total_points = atoi(argv[1]);
    int num_threads = atoi(argv[2]);

    estimate_pi(total_points, num_threads);

    return 0;
}