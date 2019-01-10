#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "input.h"
#include "effector.h"

class effector_t;

class scheduler_t {
private:
    bool is_running;
    bool is_accepting;
    void run();

public:
    scheduler_t(input_t * input);

    void submit(effector_t * effector);

    void start();
    void pause();
    void halt();
};

#endif