#include "physics.h"

#include <thread>
#include <chrono>

#include "core/constant.h"

physics_t::physics_t(){
    is_running = false;
}

physics_t::~physics_t(){
    is_running = false;

    if (thread.joinable()){
        thread.join();
    }
}

void
physics_t::run(){
    auto last_update = std::chrono::high_resolution_clock::now();

    while (is_running){
        // limit on max framerate
        auto now = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::seconds>(now - last_update).count();
        if (delta < constant::iota / 2){
            // TODO: sleep until next tick should be done
        }
        last_update = now;

        // check for collisions
        perform_collision_check();
    }
}

void
physics_t::perform_collision_check(){
    // TODO: this is O(n^2)
    //       using an octree, i think it can be reduced to O(n log(n))
    for (int i = 0; i < colliders.size(); i++){
        for (int j = i + 1; j < colliders.size(); j++){
            colliders[i].collide(colliders[j]);
        }
    }
}

void 
physics_t::start(){
    if (!is_running){
        is_running = true;


    }
}