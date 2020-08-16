#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <chrono>
#include <functional>
#include <queue>

namespace scheduler {
    using clock_t = std::chrono::high_resolution_clock;

    struct task_t {
        typedef std::function<void()> function_t;

        clock_t::time_point t;
        function_t f;

        struct comparator_t {
            bool operator()(const task_t & a, const task_t & b);
        };
    };

    namespace {
        std::priority_queue<task_t, std::vector<task_t>, task_t::comparator_t> task_queue;
    }

    template<class F>
    void schedule_at(clock_t::time_point t, const F & f){
        task_queue.push({ t, task_t::function_t(f) });
    }

    template<class F>
    void schedule_after(double delta, const F & f){
        auto t = clock_t::now() + std::chrono::microseconds(static_cast<uint32_t>(1000000 * delta));
        schedule_at(t, f);
    }

    template<class F>
    void schedule(const F & f){
        schedule_at(clock_t::now(), f);
    }
}

#endif