#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace constant {
    // smallest distance in engine = 10um
    constexpr double epsilon = 0.00001;

    // maximum render rate = 60 FPS
    // note: maximum update rate is double this (Nyquist sampling thm.)
    constexpr double iota = 0.1666666666666666;

    // relationship between a circle and its diameter
    constexpr double pi = 3.141592653589793238462643;

    // render distance = 10km
    constexpr double rho = 10000;
}

#endif