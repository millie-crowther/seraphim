#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "input.h"
#include <queue>
#include <vector>
#include <thread>
#include <functional>

class scheduler_t {
private:
    bool is_running;

    // static fields
    bool is_initialised = false;
    const int num_threads = 4;

    std::vector<std::thread> thread_pool;
    std::queue<std::function<void(void)>> tasks;

public:
    scheduler_t(){
        is_running = true;

        if (!is_initialised){
            for (int i = 0; i < num_threads; i++){
                thread_pool.push_back([&](){
                    while (is_running){
                        if (tasks.empty()){
                            // TODO: sleep until tasks available
                        } else {
                            auto task = tasks.pop();
                            task();
                        }
                    }
                });
            }

            is_initialised = true;
        }
    }

    ~scheduler_t(){
        is_running = false;
    }

    template<class effector_t>
    void submit(const effector_t effector){
        if (is_running){
            tasks.push([this](){
                effector(this);
            })
        }
    }
};

#endif