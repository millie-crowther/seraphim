#include "logic/scheduler.h"

const int num_threads = 1;
std::vector<std::thread> thread_pool;

namespace scheduler {
   namespace {
       task_t::task_t(
           const std::function<void(void)> & f,
           const task_t::time_point_t & t
       ){
           this->f = f;
           this->t = t;
       }

       bool 
       task_t::comparator_t::operator()(const task_t & t1, const task_t & t2){
           return t1.t > t2.t;
       }
   }

   void 
   halt(){
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
    start(){
        if (!is_running){
            is_running = true;

            for (int i = 0; i < num_threads; i++){
                thread_pool.push_back(std::thread([&](){
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
                }));
            }
        }
    }
}
