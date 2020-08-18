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
            period = p;
        }

        struct comparator_t {
            bool operator()(const task_t & a, const task_t & b){
                return a.t > b.t;
            }
        };
    };

    namespace {
        bool quit = true;
        std::priority_queue<task_t, std::vector<task_t>, task_t::comparator_t> task_queue;
        std::vector<std::thread> threads;
        std::condition_variable cv;
        std::mutex cv_mutex;
        std::mutex task_queue_mutex;

        void thread_pool_function(){
            while (!quit){
                bool is_queue_empty;
                bool is_task_ready = false;
                scheduler::task_t task;

                {
                    std::lock_guard<std::mutex> task_queue_lock(task_queue_mutex);
                    is_queue_empty = task_queue.empty();
                    
                    if (!is_queue_empty){
                        task = task_queue.top();
                        if (scheduler::clock_t::now() >= task.t){
                            is_task_ready = true;
                            task_queue.pop();

                            if (task.is_repeatable){
                                task.t += task.period;
                                
                                task_queue.push(task);
                            }
                        }
                    }
                }

                std::unique_lock<std::mutex> cv_lock(cv_mutex);
                if (is_queue_empty){
                    cv.wait(cv_lock);
                } else if (is_task_ready){
                    (*task.f)();
                } else {
                    cv.wait_until(cv_lock, task.t);
                }
            }

            std::cout << "Auxiliary thread terminating." << std::endl;
        }

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

            if (!quit){
                std::lock_guard<std::mutex> task_queue_lock(task_queue_mutex);
                task_queue.emplace(t, _f, is_repeatable, p);
            }

            cv.notify_one();
            return packed->get_future();
        }
    }

    void initialise(){
        quit = false;

        for (uint32_t thread = 0; thread < number_of_threads; thread++){
            threads.emplace_back(thread_pool_function);
        }
    }

    void terminate(){
        quit = true;
        cv.notify_all();

        for (auto & thread : threads){
            if (thread.joinable()){
                thread.join();
            }
        }
    }

    template<typename F, typename... Rest>
    auto schedule_at(const clock_t::time_point & t, F && f, Rest &&... rest) -> std::future<decltype(f(rest...))> {
        return schedule_task(t, false, 0s, std::forward<F>(f), std::forward<Rest>(rest)...);
    }

    template<typename F, typename... Rest, typename P>
    void schedule_every(const P & p, F && f, Rest &&... rest){
        schedule_task(clock_t::now(), true, p, std::forward<F>(f), std::forward<Rest>(rest)...);
    }

    template<typename D, typename F, typename... Rest>
    auto schedule_after(const D & d, F && f, Rest &&... rest) -> std::future<decltype(f(rest...))> {
        return schedule_at(clock_t::now() + d, std::forward<F>(f), std::forward<Rest>(rest)...);
    }
}

#endif