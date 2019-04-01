#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace constant {
    // smallest distance in engine = 10um
    constexpr double epsilon = 0.00001;

    // shortest time period recognised by the engine = 8.3 ms = 120 Hz
    constexpr double iota    = 0.00833333333333333333333;

    // relationship between a circle and its diameter
    constexpr double pi      = 3.14159265358979323846264;

    // render distance = 10km
    constexpr double rho     = 10000;
}

#endif