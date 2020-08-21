#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

// this file is partially modified from:
// https://github.com/vit-vit/CTPL

using namespace std::chrono_literals;

namespace scheduler {
    constexpr uint32_t number_of_threads = 2;

    using clock_t = std::chrono::high_resolution_clock;

    struct task_t {
        typedef std::shared_ptr<std::function<void()>> function_t;

        clock_t::time_point t;
        std::shared_ptr<std::function<void()>> f;
        bool is_repeatable;
        std::chrono::microseconds period;

        task_t(){}

        template<typename P>
        task_t(const clock_t::time_point & t, std::shared_ptr<std::function<void()>> f, bool is_repeatable, const P & p){
            this->t = t;
            this->f = f;
            this->is_repeatable = is_repeatable;
            period = std::chrono::duration_cast<std::chrono::microseconds>(p);
        }

        struct comparator_t {
            bool operator()(const task_t & a, const task_t & b);
        };
    };

    namespace __private {
        void enqueue_task(const task_t & t);

        template<typename F, typename... Rest, typename P>
        auto schedule_task(const clock_t::time_point & t, bool is_repeatable, const P & p, F && f, Rest &&... rest) -> std::future<decltype(f(rest...))> {
            auto packed = std::make_shared<std::packaged_task<decltype(f(rest...))()>>(
                std::bind(std::forward<F>(f), std::forward<Rest>(rest)...)
            );

            auto _f = std::make_shared<std::function<void()>>([packed, is_repeatable](){ 
                (*packed)(); 

                if (is_repeatable){
                    packed->reset();
                }
            });

            enqueue_task(task_t(t, _f, is_repeatable, p));
            return packed->get_future();
        }
    }

    void initialise();
    void terminate();

    template<typename F, typename... Rest>
    auto schedule_at(const clock_t::time_point & t, F && f, Rest &&... rest) -> std::future<decltype(f(rest...))> {
        return __private::schedule_task(t, false, 0s, std::forward<F>(f), std::forward<Rest>(rest)...);
    }

    template<typename F, typename... Rest, typename P>
    void schedule_every(const P & p, F && f, Rest &&... rest){
        __private::schedule_task(clock_t::now(), true, p, std::forward<F>(f), std::forward<Rest>(rest)...);
    }

    template<typename D, typename F, typename... Rest>
    auto schedule_after(const D & d, F && f, Rest &&... rest) -> std::future<decltype(f(rest...))> {
        return __private::schedule_task(clock_t::now() + d, false, 0s, std::forward<F>(f), std::forward<Rest>(rest)...);
    }
}

#endif
