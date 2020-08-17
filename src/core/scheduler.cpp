#include "core/scheduler.h"

#include <condition_variable>
#include <mutex>
#include <set>
#include <thread>
#include <queue>

std::priority_queue<scheduler::task_t, std::vector<scheduler::task_t>, scheduler::task_t::comparator_t> task_queue;
std::vector<std::thread> threads;
bool quit = true;

std::condition_variable cv;
std::mutex cv_mutex;
std::mutex task_queue_mutex;

void 
thread_pool_function(){
    while (!quit){
        bool is_queue_empty;
        scheduler::task_t task;

        {
            std::lock_guard<std::mutex> task_queue_lock(task_queue_mutex);
            is_queue_empty = task_queue.empty();
            
            if (!is_queue_empty){
                task = task_queue.top();
                task_queue.pop();
            }
        }

        std::unique_lock<std::mutex> cv_lock(cv_mutex);
        if (is_queue_empty){
            cv.wait(cv_lock);
        } else if (scheduler::clock_t::now() >= task.t){
            task.f();
        } else {
            cv.wait_until(cv_lock, task.t);
        }
    }
}

void 
scheduler::initialise(){
    quit = false;

    for (int thread = 0; thread < number_of_threads; thread++){
        threads.emplace_back(thread_pool_function);
    }
}

void
scheduler::terminate(){
    quit = true;
    cv.notify_all();
}

bool
scheduler::task_t::comparator_t::operator()(const scheduler::task_t & a, const scheduler::task_t & b){
    return a.t > b.t;
}

void 
scheduler::schedule_at(clock_t::time_point t, const scheduler::task_t::function_t & f){
    if (!quit){
        task_queue.push({ t, f });
        cv.notify_one();
    }
}

void 
scheduler::schedule(const scheduler::task_t::function_t & f){
    scheduler::schedule_at(scheduler::clock_t::now(), f);
}