#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <chrono>
#include <mutex>

#include "core/constant.h"

namespace scheduler {
    namespace {
        bool is_running = false;

        //std::mutex task_lock;
     
        struct task_t {
            std::function<void(void)> f;
            std::chrono::time_point<std::chrono::high_resolution_clock> t; 

            task_t(
                const std::function<void(void)> & f,
                const std::chrono::time_point<std::chrono::high_resolution_clock> & t 
            );

            struct comparator_t {
                bool operator()(const task_t& t1, const task_t& t2);
            };
        };

        std::priority_queue<task_t, std::vector<task_t>, task_t::comparator_t> tasks;
    }

    void start();
    void halt();

    template<class effector_t>
    void submit_after(const effector_t & effector, double t){
        if (is_running){
            //task_lock.lock();
            tasks.emplace(
                effector,
                std::chrono::high_resolution_clock::now() + std::chrono::seconds(t)
            );
            //task_lock.unlock();
        }
    }
    
    template<class effector_t>
    void submit(const effector_t & effector){
        // TODO: possibly more sophisticated scheduling here?
        submit_after(effector, constant::iota / 2.0);
    }
}

#endif
