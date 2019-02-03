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

        using task_clock = std::chrono::high_resolution_clock;
     
        struct task_t {
            typedef std::chrono::time_point<task_clock, std::chrono::microseconds> time_point_t;

            std::function<void(void)> f;
            time_point_t t; 

            task_t(
                const std::function<void(void)> & f,
                const time_point_t & t 
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
            task_t::time_point_t time_point = std::chrono::time_point_cast<task_t::time_point_t::duration>(task_clock::time_point(task_clock::now()));
            // time_point += std::chrono::seconds(t);
            tasks.emplace(
                effector,
                time_point
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
