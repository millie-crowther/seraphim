#ifndef HYPER_H
#define HYPER_H

#include <chrono>
#include <stdint.h>

using namespace std::chrono_literals;

namespace srph { namespace constant {
    // smallest distance
    constexpr double epsilon = 1.0 / 256.0;        

    constexpr double kappa   = 1.0 / 4.0;   // physics epsilon

    // shortest time period 
    constexpr auto iota    = 10ms; 

    // largest distance
    constexpr double rho     = 2048;             // metres    
    
    constexpr double pi      = 3.14159265358979323;             
}}

#endif
