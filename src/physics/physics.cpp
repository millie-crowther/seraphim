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
    using clock = std::chrono::high_resolution_clock;

    auto last_update = clock::now();
    while (is_running){
        // limit on max framerate
        auto now = clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::seconds>(now - last_update).count();
        if (delta < constant::iota){
            // TODO: sleep until next tick should be done
        }
        last_update = clock::now();

        // check for collisions
        // perform_collision_check();
    }
}

void
physics_t::collision_check(){
    std::vector<std::shared_ptr<collider_t>> cs;
    
    // lock colliders list
    for (auto & collider_ptr : colliders){
        if (auto collider = collider_ptr.lock()){
            if (collider->is_colliding()){
                cs.push_back(collider);
            }
        }
    }
    // TODO not sure this line works but youd think it would???
    std::remove_if(colliders.begin(), colliders.end(), &std::weak_ptr<collider_t>::expired);
    // unlock colliders list

    cartesian_collision_check(cs); // TODO: use planar check
}

void 
physics_t::cartesian_collision_check(const std::vector<std::shared_ptr<collider_t>> & cs) const {
    // O(n^2) complexity
    for (int i = 0; i < cs.size(); i++){
        for (int j = i + 1; j < cs.size(); j++){
            cs[i]->collide(cs[j]);
        }
    }
}

void
physics_t::planar_collision_check(const std::vector<std::shared_ptr<collider_t>> & cs) const {
    // O(n * log(n)) complexity
    
    // check for base cases
    if (cs.size() <= 1){
        return;
    } else if (cs.size() == 2){
        cs[0]->collide(cs[1]);
    } else if (cs.size() <= 4){
        cartesian_collision_check(cs);
    }

    // calculate mean and variance of collider centres
    vec3_t mean;
    for (const auto & collider : cs){
        mean += collider->get_centre();
    }
    mean /= cs.size();

    vec3_t variance;
    for (const auto & collider : cs){
        vec3_t centre = collider->get_centre();
        for (int i = 0; i < 3; i++){
            variance[i] += (centre[i] - mean[i]) * (centre[i] - mean[i]);
        }
    }
    variance = variance.normalise();

    // divide and conquer
    std::vector<std::shared_ptr<collider_t>> a;
    std::vector<std::shared_ptr<collider_t>> b;

    bool is_degenerate = true; // tee-hee 
    for (auto & collider : cs){
        if (collider->intersects_plane(mean, variance)){ 
            a.push_back(collider);
            b.push_back(collider);
        } else {
            is_degenerate = false;

            if ((collider->get_centre() - mean) * variance > 0){
                a.push_back(collider);
            } else {
                b.push_back(collider);
            }
        }
    }

    if (is_degenerate){
        cartesian_collision_check(cs);
    } else {
        planar_collision_check(a);
        planar_collision_check(b);
    }
}

void 
physics_t::start(){
    if (!is_running){
        is_running = true;

        thread = std::thread(std::bind(&physics_t::run, this));
    }
}