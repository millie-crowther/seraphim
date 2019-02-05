#ifndef PHYSICS_H
#define PHYSICS_H

#include <vector>
#include <memory>

#include "collider.h"

class physics_t {
private:
    std::vector<std::weak_ptr<collider_t>> colliders;

    void run();

    void collision_check();

    // O(n^2)
    void cartesian_collision_check(const std::vector<std::shared_ptr<collider_t>> & cs) const; 

    // O(n * log(n))
    void planar_collision_check(const std::vector<std::shared_ptr<collider_t>> & cs) const;
    
    bool is_running;
    std::thread thread;

public:
    physics_t();
    ~physics_t();
    void start();
};

#endif