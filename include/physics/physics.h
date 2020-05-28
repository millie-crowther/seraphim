#ifndef PHYSICS_H
#define PHYSICS_H

#include <thread>

class physics_t {
private:
    bool quit;
    std::thread thread;

    void tick(double delta);
    void run();

public:
    physics_t();
    ~physics_t();
};


#endif