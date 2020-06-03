#ifndef HYPER_H
#define HYPER_H

#include <stdint.h>

namespace hyper {
    // smallest distance
    constexpr double epsilon = 0.0078125;             // 2^-7 metres

    // shortest time period 
    constexpr double iota    = 1.0 / 60.0;                // Hertz

    // largest distance
    constexpr double rho     = 2048;                 // metres    

    // largest object
    constexpr double kappa   = 256; 
}

#endif