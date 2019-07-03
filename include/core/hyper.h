#ifndef HYPER_H
#define HYPER_H

#include <stdint.h>

namespace hyper {
    // smallest distance
    constexpr double   epsilon = 0.001;             // metres

    // shortest time period 
    constexpr double   iota    = 60;                // Hertz

    // node size
    constexpr double   sigma   = 80 * epsilon;     // metres      

    // largest distance
    constexpr double   rho     = 20;                 // metres

    // number of bricks
    constexpr uint32_t tau     = 256;               
}

#endif