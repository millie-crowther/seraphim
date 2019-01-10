#ifndef EFFECTOR_H
#define EFFECTOR_H

#include <chrono>
#include <memory>
#include <functional>

#include "scheduler.h"

class scheduler_t;

template<class effector_func_t>
class effector_t {
private:
    // private fields
    effector_func_t f;

public:
    effector_t(const effector_func_t& f){
        this->f = f;
    }

    void run(const scheduler_t * scheduler){
        f(scheduler);
        scheduler->effector_complete();
    }
};

#endif
