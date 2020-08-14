#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <chrono>
#include <functional>

namespace scheduler {
    using clock_t = std::chrono::high_resolution_clock;

    struct task_t {
        clock_t::time_point t;
        std::function<void()> f;

        struct comparator_t {
            bool operator()(const task_t & a, const task_t & b);
        };
    };

    void schedule(clock_t::time_point t, std::function<void()> f);
}

#endif