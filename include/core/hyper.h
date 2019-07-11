#ifndef HYPER_H
#define HYPER_H

#include <stdint.h>

namespace hyper {
    // smallest distance
    constexpr double   epsilon = 0.0078125;             // 2^-7 metres

    // shortest time period 
    constexpr double   iota    = 60;                // Hertz

    // largest distance
    constexpr double   rho     = 256;                 // metres

    // number of bricks
    constexpr uint32_t tau     = 256;               
}

#endif