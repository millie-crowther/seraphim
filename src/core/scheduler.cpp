#include "core/scheduler.h"

bool quit = true;
std::vector<std::thread> threads;
std::mutex cv_mutex;
std::priority_queue<
    scheduler::__private::task_t, 
    std::vector<scheduler::__private::task_t>, 
    scheduler::__private::task_t::comparator_t
> task_queue;
std::condition_variable cv;
std::mutex task_queue_mutex;

scheduler::__private::task_t::task_t(){}

void
scheduler::__private::enqueue_task(const task_t & t){
    if (!quit){
        std::lock_guard<std::mutex> task_queue_lock(task_queue_mutex);
        task_queue.emplace(t);
    }
    cv.notify_one();
}

void 
thread_pool_function(){
    while (!quit){
        bool is_queue_empty;
        bool is_task_ready = false;
        scheduler::__private::task_t task;

        {
            std::lock_guard<std::mutex> task_queue_lock(task_queue_mutex);
            is_queue_empty = task_queue.empty();
            
            if (!is_queue_empty){
                task = task_queue.top();
                if (scheduler::clock_t::now() >= task.t){
                    is_task_ready = true;
                    task_queue.pop();

                    if (task.is_repeatable && !quit){
                        task_queue.emplace(
                            task.t + task.period, 
                            task.f,
                            task.is_repeatable,
                            task.period
                        );
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

scheduler::__private::task_t::task_t(const clock_t::time_point & t, std::shared_ptr<std::function<void()>> f, bool is_repeatable, const clock_t::duration & period){
    this->t = t;
    this->f = f;
    this->is_repeatable = is_repeatable;
    this->period = period;
}

void 
scheduler::initialise(){
    quit = false;

    for (uint32_t thread = 0; thread < number_of_threads; thread++){
        threads.emplace_back(thread_pool_function);
    }
}

void 
scheduler::terminate(){
    quit = true;
    cv.notify_all();

    for (auto & thread : threads){
        if (thread.joinable()){
            thread.join();
        }
    }
}

bool
scheduler::__private::task_t::comparator_t::operator()(const scheduler::__private::task_t & a, const scheduler::__private::task_t & b){
    return a.t > b.t;
}
