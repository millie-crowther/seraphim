#ifndef PHYSICS_H
#define PHYSICS_H

#include <list>
#include <memory>

#include "collider.h"

class physics_t {
private:
    std::list<std::weak_ptr<collider_t>> colliders;

    void run();

    // collision checking functions
    void collision_check();
    void cartesian_collision_check(const std::vector<std::shared_ptr<collider_t>> & cs) const; 
    void planar_collision_check(const std::vector<std::shared_ptr<collider_t>> & cs) const;
    
    bool is_running;
    std::thread thread;

public:
    physics_t();
    ~physics_t();
    void start();
};

#endif