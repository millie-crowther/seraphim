#include "physics/physics.h"

#include "core/scheduler.h"

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

    for (int i = 0; i < max_iterations && fx > constant::epsilon; i++){
        x -= dfdx(x) * std::abs(fx);
        fx = f(x);
    }

    const double CoR = 0.9;
    
    if (fx > constant::epsilon){
         return;
    }

    std::cout << "collision detected!" << std::endl;

    auto n = a->phi(x) > b->phi(x) ? a->normal(x) : b->normal(x);
    
    auto va = a->get_local_velocity(x);
    auto ra = a->get_offset_from_centre_of_mass(x);
    auto ia = mat::inverse(*a->get_inertia_tensor());
    auto xa = vec::cross(ia * vec::cross(ra, n), ra); 
    auto ma = 1.0 / a->get_mass();

    auto vb = b->get_local_velocity(x);
    auto rb = b->get_offset_from_centre_of_mass(x);
    auto ib = mat::inverse(*b->get_inertia_tensor());
    auto xb = vec::cross(ib * vec::cross(rb, n), rb);
    auto mb = 1.0 / b->get_mass();

    double j = 
        -(1.0 + CoR) * vec::dot(vb - va, n) /
        (ma + mb + vec::dot(xa + xb, n));

    std::cout << j << std::endl;
}
