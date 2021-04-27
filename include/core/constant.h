#ifndef SERAPHIM_CONSTANT_H
#define SERAPHIM_CONSTANT_H

// smallest distance (metres)
// if two positions are less than epsilon apart, they are in the same position
static const double epsilon = 1.0 / 300.0;

// physics time step (seconds)
static const double sigma = 0.01;

// smallest possible duration (seconds)
// if two events occur less than iota apart, they happened at the same time
static const double iota = 0.001;

// largest distance (metres)
static const double rho = 2048;

// ratio of a circle's diameter to its circumference
static const double pi = 3.14159265358979323;

#endif
