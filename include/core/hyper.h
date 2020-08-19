#ifndef HYPER_H
#define HYPER_H

#include <chrono>
#include <stdint.h>

using namespace std::chrono_literals;

namespace hyper {
    // smallest distance
    constexpr double epsilon = 1.0 / 256.0;      // metres    

    // shortest time period 
    constexpr auto iota    = 16.66666ms; // i.e. 60 Hz

    // largest distance
    constexpr double rho     = 2048;             // metres    
    
    constexpr double pi      = 3.14159265358979323;             
}

#endif