#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <chrono>
#include <mutex>

namespace scheduler_t {
    bool is_running = false;
    bool is_initialised = false;

    const int num_threads = 4;

    const double minimum_update_period = 0.16666; // 60 FPS

    std::vector<std::thread> thread_pool;
    std::mutex task_lock;
 
    struct task_t {
        std::function<void(void)> f;
        std::chrono::time_point<std::chrono::high_resolution_clock> t; 

        struct comparator_t {
            bool operator()(const task_t& t1, const task_t& t2){
                return t1.t > t2.t;
            }
        };
    };

    std::priority_queue<task_t, std::vector<task_t>, task_t::comparator_t> tasks;

    void start(){
        if (!is_initialised){
            is_running = true;

            for (int i = 0; i < num_threads; i++){
                thread_pool.push_back(std::thread([&](){
                    while (is_running){
                        task_lock.lock();
                        if (tasks.empty()){
                            task_lock.unlock();
                            // TODO: sleep until tasks available
                        } else {
                            auto task = tasks.top();
                            // TODO: if first task is scheduled for a while, sleep
                            tasks.pop();
                            task_lock.unlock();
                            task.f();
                        }
                    }
                }));
            }

            is_initialised = true;
        }
    }

    void halt(){
        is_running = false;

        task_lock.lock();
        while (!tasks.empty()){
            tasks.pop();
        }
        task_lock.unlock();

        for (auto& thread : thread_pool){
            thread.join(); 
        }

        thread_pool.clear();
    }

    template<class effector_t>
    void submit(const effector_t effector){
        submit_after(effector, minimum_update_period);
    }

    template<class effector_t>
    void submit_after(const effector_t effector, double t){
        if (is_running){
            task_lock.lock();
            tasks.push({
                [=](){ effector(); },
                std::chrono::high_resolution_clock::now() + std::chrono::seconds(t)
            });
            task_lock.unlock();
        }
    }
}

#endif