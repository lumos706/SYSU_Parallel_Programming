#pragma once
#include <pthread.h>

// 调度策略枚举
enum class SchedulePolicy {
    Static,
    Dynamic,
    Guided
};

typedef void (*parallel_func)(int, void*);

extern "C" {
    void parallel_for(int start, int end, int inc,
                     parallel_func functor, void* arg,
                     int num_threads, SchedulePolicy policy);
}