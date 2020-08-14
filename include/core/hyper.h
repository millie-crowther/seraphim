#ifndef HYPER_H
#define HYPER_H

#include <stdint.h>

namespace hyper {
    // smallest distance
    constexpr double epsilon = 1.0 / 256.0;      // metres    

    // shortest time period 
    constexpr double iota    = 1.0 / 60.0;       // Hertz

    // largest distance
    constexpr double rho     = 2048;             // metres    
    
    constexpr double pi      = 3.14159265358979323;             
}

#endif