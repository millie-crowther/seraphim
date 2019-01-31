#ifndef PHYSICS_H
#define PHYSICS_H

#include <vector>

#include "rigidbody.h"

class physics_t {
private:
    std::vector<rigidbody_t> rigidbodies;

public:
    void start();
    void halt();
};

#endif