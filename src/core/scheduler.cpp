#include "core/scheduler.h"

#include <condition_variable>
#include <mutex>
#include <set>
#include <thread>
#include <queue>

std::priority_queue<scheduler::task_t, std::vector<scheduler::task_t>, scheduler::task_t::comparator_t> task_queue;
std::set<std::thread> threads;
bool quit = true;

std::condition_variable cv;
std::mutex task_queue_mutex;
std::lock_guard<std::mutex> task_queue_lock;

void 
thread_pool_function(){
    while (!quit){

    }
}

void 
scheduler::initialise(){
    quit = false;

    for (int thread = 0; thread < number_of_threads; thread++){

    }
}

void
scheduler::terminate(){
    quit = true;
}

bool
scheduler::task_t::comparator_t::operator()(const scheduler::task_t & a, const scheduler::task_t & b){
    return a.t > b.t;
}

void 
scheduler::schedule_at(clock_t::time_point t, const scheduler::task_t::function_t & f){
    if (!quit){
        task_queue.push({ t, f });
    }
}

void 
scheduler::schedule_after(double delta, const scheduler::task_t::function_t & f){
    auto t = scheduler::clock_t::now() + std::chrono::microseconds(static_cast<uint32_t>(1000000 * delta));
    scheduler::schedule_at(t, f);
}

void 
scheduler::schedule(const scheduler::task_t::function_t & f){
    scheduler::schedule_at(scheduler::clock_t::now(), f);
}