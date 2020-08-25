#include "physics/physics.h"

#include "core/scheduler.h"
#include "substance/matter/sdf/compose.h"

#include <chrono>
#include <functional>
#include <iostream>

physics_t::physics_t(){
    quit = false;
    thread = std::thread(&physics_t::run, this);
}

physics_t::~physics_t(){
    quit = true;
    thread.join();
}

void 
physics_t::run(){
    auto t = scheduler::clock_t::now();
    auto clock_d = std::chrono::duration_cast<scheduler::clock_t::duration>(constant::iota);
    double physics_d = constant::iota.count() / 1000.0;

    while (!quit){
        for (auto & m : matters){
            m->physics_tick(physics_d);
        }

        for (auto a_it = matters.begin(); a_it != matters.end(); a_it++){
            for (auto b_it = std::next(a_it); b_it != matters.end(); b_it++){
                collide(*a_it, *b_it);
            }
        }

        t += clock_d;
        std::this_thread::sleep_until(t);
    }
}

void 
physics_t::register_matter(std::shared_ptr<matter_t> matter){
    matters.insert(matter);
}
    
void 
physics_t::unregister_matter(std::shared_ptr<matter_t> matter){
    auto it = matters.find(matter);
    if (it != matters.end()){
        matters.erase(it);
    }
}

void
physics_t::collide(std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b){
    static const int max_iterations = 10;

    auto f = [a, b](const vec3_t & x){
        return std::max(a->phi(x), b->phi(x));
    };

    auto dfdx = [f](const vec3_t & x){
        return vec::grad(f, x);
    }; 
    
    auto x = (a->get_position() + b->get_position()) / 2.0;
    auto fx = f(x);

    bool is_colliding = false;

    for (int i = 0; i < max_iterations && !is_colliding; i++){
        x -= dfdx(x) * std::abs(fx);
        fx = f(x);
        is_colliding = fx < constant::epsilon;
    }
}
