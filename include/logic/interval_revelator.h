#ifndef INTERVAL_REVELATOR_H
#define INTERVAL_REVELATOR_H

#include <chrono>

#include "logic/revelator.h"

class interval_revelator_t : public revelator_t<double> {
private:
    std::chrono::high_resolution_clock::time_point previous;

public:
    interval_revelator_t();

    void tick();
};

#endif