#ifndef PHYSICS_H
#define PHYSICS_H

#include <thread>

#include "core/hyper.h"

class physics_t {
private:
    bool quit;
    std::thread thread;

    void tick();
    void run();

public:
    physics_t();
    ~physics_t();
};


#endif