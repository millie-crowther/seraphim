#ifndef EFFECTOR_H
#define EFFECTOR_H

#include "input.h"
#include <chrono>

class effector_t {
private:
    // private fields
    std::chrono::high_resolution_clock::time_point last_tick;

protected:
    // template method 
    virtual void tick(float delta, const input_t& input);

public:
    // constructor
    effector_t();

    // public method
    void tick_template(const input_t& input);
};

#endif