#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "input.h"
#include <queue>

class scheduler_t {
private:
    bool is_running;
    std::atomic<int> running_effectors;

    // static fields
    static bool is_initialised;

public:
    scheduler_t(){
        is_running = true;
        running_effectors.store(0);
    }

    template<class effector_t>
    void submit(const effector_t effector){
        if (is_running){
            effector(this);
            running_effectors++;
        }
    }

    // notifies the scheduler that an effector has completed its action
    void effector_complete(){
        running_effectors--;

        if (!is_running && running_effectors.load() <= 0){
            //notify all
        }
    }

    void halt(){
        is_running = false;

        // wait for running effectors to finish
    }
};

#endif