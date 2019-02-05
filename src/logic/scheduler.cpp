#include "logic/scheduler.h"

#include <vector>
#include <queue>
#include <thread>
#include <chrono>

#include "core/constant.h"

using clock = std::chrono::high_resolution_clock;

static bool is_finished = false;
static bool is_initialised = false;
static constexpr int num_threads = 1;
static std::vector<std::thread> thread_pool;   

struct task_t {
    typedef std::chrono::time_point<clock, std::chrono::microseconds> time_point_t;
    time_point_t t;

    std::function<void(void)> f;

    task_t(const std::function<void(void)> & f, double delay){
        this->f = f;
        t = std::chrono::time_point_cast<time_point_t::duration>(clock::now());
        // t += std::chrono::seconds(delay);
    }

    struct comparator_t {
        bool operator()(const task_t& t1, const task_t& t2){
            return t1.t > t2.t;
        }
    };
};

static std::priority_queue<task_t, std::vector<task_t>, task_t::comparator_t> tasks;

//
// private functions
//
static void
thread_func(){
    while (!is_finished){
        //task_lock.lock();
        if (tasks.empty()){
            //task_lock.unlock();
            // TODO: sleep until tasks available
        } else {
            auto task = tasks.top();
            // TODO: if first task is scheduled for a while, sleep
            
            if (task.t >= clock::now()){
                tasks.pop();
                //task_lock.unlock();
                task.f();
            }
        }
    }
}

static void 
start(){
    if (!is_finished && !is_initialised){
        for (int i = 0; i < num_threads; i++){
            thread_pool.push_back(std::thread(thread_func));
        }
        is_initialised = true;
    }
}

//
// public functions
//
void 
scheduler::halt(){
    is_finished = true;

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
    if (!is_initialised){
        start();
    }

    if (!is_finished){
        //task_lock.lock();
        tasks.emplace(effector, delay);
        //task_lock.unlock();
    }
}

void 
scheduler::submit(const effector_t & effector){
    submit_after(effector, constant::iota);
}