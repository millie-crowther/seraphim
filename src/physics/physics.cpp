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
        for (auto a_it = matters.begin(); a_it != matters.end(); a_it++){
            for (auto b_it = std::next(a_it); b_it != matters.end(); b_it++){
                collide(*a_it, *b_it);
            }
        }
        
        for (auto & m : matters){
            m->physics_tick(physics_d);
            if (m->get_position()[1] < -100.0){
                m->get_transform().set_position(vec3_t(0.0, -100.0, 0.0));
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

    // detect collision
    auto f = [a, b](const vec3_t & x){
        return std::max(a->phi(x), b->phi(x));
    };

    auto dfd = [f](const vec3_t & x){
        return vec::grad(f, x);
    }; 
    
    auto x = (a->get_position() + b->get_position()) / 2.0;
    auto fx = f(x);
    auto dfdx = dfd(x);

    for (int i = 0; i < max_iterations && fx > 0; i++){
        x -= dfdx * std::abs(fx);
        fx = f(x);
        dfdx = dfd(x);
    }

    if (fx > 0){
         return;    
    }

    // extricate matters
    auto n = dfdx;
    
    auto sm = a->get_mass() + b->get_mass();
    double da = fx * b->get_mass() / sm;
    double db = fx * a->get_mass() / sm;
    a->get_transform().translate(-da * n);
    b->get_transform().translate( db * n);     
 
    // update velocities
    auto va = a->get_velocity(x);
    auto ra = a->get_offset_from_centre_of_mass(x);
    auto ia = mat::inverse(a->get_inertia_tensor());
    auto xa = vec::cross(ia * vec::cross(ra, n), ra); 
    auto ma = 1.0 / a->get_mass();
    auto ta = a->get_material(x); // TODO: need to move into local coordinates

    auto vb = b->get_velocity(x);
    auto rb = b->get_offset_from_centre_of_mass(x);
    auto ib = mat::inverse(b->get_inertia_tensor());
    auto xb = vec::cross(ib * vec::cross(rb, n), rb);
    auto mb = 1.0 / b->get_mass();
    auto tb = b->get_material(x);

    double CoR = std::max(ta.restitution, tb.restitution);
 
    double j = 
        -(1.0 + CoR) * vec::dot(vb - va, n) /
        (ma + mb + vec::dot(xa + xb, n));

    vec3_t dva = -j * n / a->get_mass();
    vec3_t dwa = -j * ia * vec::cross(ra, n); 
    a->update_velocities(dva, dwa);

    vec3_t dvb = j * n / b->get_mass();
    vec3_t dwb = j * ib * vec::cross(rb, n); 
    b->update_velocities(dvb, dwb);
}
