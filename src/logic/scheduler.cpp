#include "logic/scheduler.h"

#include <vector>
#include <queue>
#include <thread>
#include <chrono>

#include "core/constant.h"

static bool is_running = false;
static constexpr int num_threads = 1;
static std::vector<std::thread> thread_pool;   

struct task_t {
    typedef std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::microseconds> time_point_t;
    time_point_t t;

    std::function<void(void)> f;

    task_t(const std::function<void(void)> & f, double delay){
        this->f = f;
        t = std::chrono::time_point_cast<time_point_t::duration>(std::chrono::high_resolution_clock::now());
        // t += std::chrono::seconds(delay);
    }

    struct comparator_t {
        bool operator()(const task_t& t1, const task_t& t2){
            return t1.t > t2.t;
        }
    };
};

static std::priority_queue<task_t, std::vector<task_t>, task_t::comparator_t> tasks;

void 
scheduler::start(){
    if (!is_running){
        is_running = true;

        auto thread_func = [&](){
            while (is_running){
                //task_lock.lock();
                if (tasks.empty()){
                    //task_lock.unlock();
                    // TODO: sleep until tasks available
                } else {
                    auto task = tasks.top();
                    // TODO: if first task is scheduled for a while, sleep
                    
                    if (task.t >= std::chrono::high_resolution_clock::now()){
                        tasks.pop();
                        //task_lock.unlock();
                        task.f();
                    }
                }
            }
        };

        for (int i = 0; i < num_threads; i++){
            thread_pool.push_back(std::thread(thread_func));
        }
    }
}

void 
scheduler::halt(){
    is_running = false;

    //task_lock.lock(); 
    while (!tasks.empty()){
        tasks.pop();
    }
    //task_lock.unlock();

    for (auto & thread : thread_pool){
        if (thread.joinable()){
            thread.join();
        }
    }

    thread_pool.clear();
}

void 
scheduler::submit_after(const effector_t & effector, double delay){
    if (is_running){
        //task_lock.lock();
        tasks.emplace(effector, delay);
        //task_lock.unlock();
    }
}

void 
scheduler::submit(const effector_t & effector){
    submit_after(effector, constant::iota / 2.0);
}