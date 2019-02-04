#include "physics.h"

#include <thread>
#include <chrono>
#include <numeric>

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
        last_update = std::chrono::high_resolution_clock::now();

        // check for collisions
        perform_collision_check();
    }
}

void
physics_t::perform_collision_check(){
    // TODO: this is O(n^2)
    //       using an octree, i think it can be reduced to O(n log(n))
    // for (int i = 0; i < colliders.size(); i++){
    //     for (int j = i + 1; j < colliders.size(); j++){
    //         colliders[i].collide(colliders[j]);
    //     }
    // }


}

void
physics_t::collision_helper(const std::vector<std::shared_ptr<collider_t>> & cs){
    vec3_t mean;
    for (auto & collider : cs){
        mean += collider->get_centre();
    }
    mean /= cs.size();

    vec3_t variance;
    for (auto & collider : cs){
        vec3_t centre = collider->get_centre();
        for (int i = 0; i < 3; i++){
            variance[i] += (centre[i] - mean[i]) * (centre[i] - mean[i]);
        }
    }
    variance = variance.normalise();

    std::vector<std::shared_ptr<collider_t>> a;
    std::vector<std::shared_ptr<collider_t>> b;

    for (auto & collider : cs){
        if ((collider->get_centre() - mean).dot(variance) > 0){
            a.push_back(collider);
        } else {
            b.push_back(collider);
        }
    }
}

void 
physics_t::start(){
    if (!is_running){
        is_running = true;


    }
}