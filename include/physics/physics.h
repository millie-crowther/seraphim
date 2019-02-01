#ifndef PHYSICS_H
#define PHYSICS_H

#include <vector>

#include "collider.h"

class physics_t {
private:
    std::vector<collider_t> colliders;

    void run();
    void tick();
    
    bool is_running;
    std::thread thread;

public:
    physics_t();
    ~physics_t();
    void start();
};

#endif