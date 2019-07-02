#ifndef HYPER_H
#define HYPER_H

#include <stdint.h>

namespace hyper {
    // smallest distance
    constexpr double   epsilon = 0.001;             // metres

    // shortest time period 
    constexpr double   iota    = 60;                // Hertz

    // patch resolution
    constexpr uint16_t pi      = 8;                 // pixels

    // patch size
    constexpr double   sigma   = 20 * epsilon * pi; // metres      

    // largest distance
    constexpr double   rho     = 8;                // metres

    // number of bricks
    constexpr uint32_t tau     = 256;               
}

#endif