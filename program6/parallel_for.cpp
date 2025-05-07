#include "parallel_for.hpp"
#include <vector>
#include <memory>
#include <atomic>
#include <cmath>

// 任务参数结构体
struct TaskArgs {
    int start;
    int end;
    int inc;
    parallel_func functor;
    void* arg;
    SchedulePolicy policy;
    std::atomic<int>* counter;  // 仅动态调度使用
    int chunk_size;             // 仅动态/guided使用
};

// 线程工作函数
void* thread_worker(void* args) {
    TaskArgs* t_args = static_cast<TaskArgs*>(args);
    
    switch(t_args->policy) {
    case SchedulePolicy::Static: {
        for(int i = t_args->start; i < t_args->end; i += t_args->inc) {
            t_args->functor(i, t_args->arg);
        }
        break;
    }
    case SchedulePolicy::Dynamic: {
        while(true) {
            int current = t_args->counter->fetch_add(t_args->chunk_size);
            if(current >= t_args->end) break;
            
            int block_end = current + t_args->chunk_size * t_args->inc;
            block_end = block_end > t_args->end ? t_args->end : block_end;
            
            for(int i = current; i < block_end; i += t_args->inc) {
                t_args->functor(i, t_args->arg);
            }
        }
        break;
    }
    case SchedulePolicy::Guided: {
        int remaining = t_args->end - t_args->start;
        while(remaining > 0) {
            int chunk = remaining / (2 * t_args->chunk_size);
            chunk = chunk < 1 ? 1 : chunk;
            
            int current = t_args->counter->fetch_add(chunk);
            if(current >= t_args->end) break;
            
            int block_end = current + chunk * t_args->inc;
            block_end = block_end > t_args->end ? t_args->end : block_end;
            
            for(int i = current; i < block_end; i += t_args->inc) {
                t_args->functor(i, t_args->arg);
            }
            remaining = t_args->end - block_end;
        }
        break;
    }
    }
    delete t_args;
    return nullptr;
}

// 主函数实现
void parallel_for(int start, int end, int inc,
                 parallel_func functor, void* arg,
                 int num_threads, SchedulePolicy policy) {
    if(start >= end || inc <= 0 || num_threads <= 0) return;

    std::vector<pthread_t> threads(num_threads);
    std::atomic<int> counter(start);
    const int total_iters = (end - start + inc - 1) / inc;
    const int base_chunk = (total_iters + num_threads - 1) / num_threads;

    for(int i = 0; i < num_threads; ++i) {
        TaskArgs* args = new TaskArgs;
        args->functor = functor;
        args->arg = arg;
        args->inc = inc;
        args->policy = policy;
        args->counter = &counter;
        args->chunk_size = base_chunk;

        // 静态调度初始化
        if(policy == SchedulePolicy::Static) {
            int chunk = (total_iters / num_threads) * inc;
            args->start = start + i * chunk;
            args->end = (i == num_threads-1) ? end : args->start + chunk;
            args->end = args->end > end ? end : args->end;
        } else {
            args->start = start;
            args->end = end;
        }

        pthread_create(&threads[i], nullptr, thread_worker, args);
    }

    for(auto& thread : threads) {
        pthread_join(thread, nullptr);
    }
}