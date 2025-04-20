#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <chrono>

// 并行数组求和
struct SumArgs {
    int start_idx;
    int end_idx;
    const std::vector<double>* array;
    double partial_sum = 0.0;
};

void array_sum(SumArgs& args) {
    double sum = 0.0;
    for (int i = args.start_idx; i < args.end_idx; i++) {
        sum += (*args.array)[i];
    }
    args.partial_sum = sum;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <array_size> <thread_num>\n";
        return 1;
    }

    int size = std::stoi(argv[1]);
    int num_threads = std::stoi(argv[2]);

    // 分配数组并填充随机数
    std::vector<double> array(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int i = 0; i < size; i++) {
        array[i] = dis(gen);
    }

    // 创建线程
    std::vector<std::thread> threads;
    std::vector<SumArgs> args(num_threads);
    int elements_per_thread = size / num_threads;

    auto start = std::chrono::high_resolution_clock::now();

    for (int t = 0; t < num_threads; t++) {
        args[t].start_idx = t * elements_per_thread;
        args[t].end_idx = (t == num_threads - 1) ? size : (t + 1) * elements_per_thread;
        args[t].array = &array;
        threads.emplace_back(array_sum, std::ref(args[t]));
    }

    // 等待线程完成并汇总结果
    long long total_sum = 0;
    for (auto& thread : threads) {
        thread.join();
    }
    for (const auto& arg : args) {
        total_sum += arg.partial_sum;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_taken = end - start;

    std::cout << size << " " << num_threads << " Total sum: " << total_sum << " Array sum time: " << time_taken.count() << " seconds\n";

    return 0;
}