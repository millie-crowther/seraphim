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
    static bool is_initialised = false;

    std::vector<std::thread> thread_pool;
    std::queue<std::function<void(void)>> tasks;

public:
    scheduler_t(){
        is_running = true;

        if (!is_initialised){

        }
    }

    template<class effector_t>
    void submit(const effector_t effector){
        if (is_running){
            tasks.push([this](){
                effector(this);
            })
        }
    }

    void halt(){
        is_running = false;

        // wait for running effectors to finish
    }
};

#endif