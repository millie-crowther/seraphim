#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "input.h"
#include "effector.h"
#include <queue>

// class effector_t;

class scheduler_t {
private:
    bool is_running;
    std::atomic<int> running_effectors;

    void run(){
        is_running = true;
    }

    // static fields
    static bool is_initialised;

public:
    scheduler_t();

    void submit(const std::shared_ptr<effector_t> effector){

    }

    // notifies the scheduler that an effector has completed its action
    void effector_complete();

    void halt();
};

#endif