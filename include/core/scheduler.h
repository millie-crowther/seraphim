#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <chrono>
#include <functional>

namespace scheduler {
    constexpr uint32_t number_of_threads = 2;

    using clock_t = std::chrono::high_resolution_clock;

    struct task_t {
        typedef std::function<void()> function_t;

        clock_t::time_point t;
        function_t f;

        struct comparator_t {
            bool operator()(const task_t & a, const task_t & b);
        };
    };

    void initialise();
    void terminate();

    void schedule_at(clock_t::time_point t, const task_t::function_t & f);
    void schedule(const task_t::function_t & f);
}

#endif