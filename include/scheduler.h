#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "input.h"
#include "effector.h"
#include <queue>

class effector_t;

class scheduler_t {
private:
    // private constructor
    scheduler_t();

    bool is_running;
    bool is_accepting;
    void run();

    // static fields
    static bool is_initialised;

    std::queue<const std::shared_ptr<effector_t>> next_frame;

public:
    void submit(const std::shared_ptr<effector_t> effector);

    void start();
    void pause();
    void halt();

    // factories
    // only shared pointers to schedulers are allowed to prevent nasty 
    // concurrency problems
    std::shared_ptr<scheduler_t> create();
};

#endif