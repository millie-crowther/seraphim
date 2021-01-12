#ifndef HYPER_H
#define HYPER_H

#include <chrono>
#include <stdint.h>

using namespace std::chrono_literals;

namespace srph { namespace constant {
    // smallest distance (metres)
    // if two positions are less than epsilon apart, they are in the same position
    constexpr double epsilon = 1.0 / 300.0;        

    // physics time step (seconds)
    constexpr double sigma = 0.01;  

    // smallest possible duration (seconds)
    // if two events occur less than iota apart, they happened at the same time
    constexpr double iota = 0.001;    

    // largest distance
    constexpr double rho     = 2048;             // metres    
    
    constexpr double pi      = 3.14159265358979323;             
}}

#endif
