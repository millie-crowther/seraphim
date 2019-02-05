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
        // perform_collision_check();
    }
}

void
physics_t::collision_check(){
    // TODO: this is O(n^2)
    //       i think it can be reduced to O(n log(n))

    std::vector<std::shared_ptr<collider_t>> cs;
    
}

void 
physics_t::cartesian_collision_check(const std::vector<std::shared_ptr<collider_t>> & cs) const {
    for (int i = 0; i < cs.size(); i++){
        for (int j = i + 1; j < cs.size(); j++){
            cs[i]->collide(*cs[j]);
        }
    }
}

void
physics_t::planar_collision_check(const std::vector<std::shared_ptr<collider_t>> & cs) const {
    // calculate mean and variance of collider centres
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

    // subdivide space
    std::vector<std::shared_ptr<collider_t>> a;
    std::vector<std::shared_ptr<collider_t>> b;

    bool is_degenerate = true; // tee-hee 
    for (auto & collider : cs){
        if (collider->intersects_plane(mean, variance)){
            a.push_back(collider);
            b.push_back(collider);
        } else {
            is_degenerate = false;

            if ((collider->get_centre() - mean).dot(variance) > 0){
                a.push_back(collider);
            } else {
                b.push_back(collider);
            }
        }
    }

    // recurse
    if (is_degenerate){
        cartesian_collision_check(cs);
    } else {
        if (a.size() == 2){
            a[0]->collide(*a[1]);
        } else if (a.size() > 2) {
            planar_collision_check(a);
        }

        if (b.size() == 2){
            b[0]->collide(*b[0]);
        } else if (b.size() > 2){
            planar_collision_check(b);
        }
    }
}

void 
physics_t::start(){
    if (!is_running){
        is_running = true;

        thread = std::thread(std::bind(physics_t::run, this));
    }
}