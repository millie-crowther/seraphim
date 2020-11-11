#include "physics/physics.h"

#include "core/scheduler.h"
#include "physics/collision.h"

#include <chrono>
#include <functional>
#include <iostream>

srph::physics_t::physics_t(){
    quit = false;
    thread = std::thread(&physics_t::run, this);
}

srph::physics_t::~physics_t(){
    quit = true;
    thread.join();
}

void 
srph::physics_t::run(){
    auto t = scheduler::clock_t::now();
    auto clock_d = std::chrono::duration_cast<scheduler::clock_t::duration>(constant::iota);
    double physics_d = constant::iota.count() / 1000.0;

    while (!quit){
        std::vector<collision_t> collisions;

        for (auto a_it = matters.begin(); a_it != matters.end(); a_it++){
            for (auto b_it = std::next(a_it); b_it != matters.end(); b_it++){
                
                collision_t c = collide(*a_it, *b_it);
                if (c.hit){
                    collisions.push_back(c);
                }
            }
        }

        std::for_each(collisions.begin(), collisions.end(), collision_correct);
        
        for (auto & m : matters){
            m->physics_tick(physics_d);
            if (m->get_position()[1] < -90.0){
                m->get_transform().set_position(vec3_t(0.0, -100.0, 0.0));
                m->reset_velocity();
            }
        }

        t += clock_d;
        std::this_thread::sleep_until(t);
    }
}

void 
srph::physics_t::register_matter(std::shared_ptr<matter_t> matter){
    matters.insert(matter);
}
    
void 
srph::physics_t::unregister_matter(std::shared_ptr<matter_t> matter){
    auto it = matters.find(matter);
    if (it != matters.end()){
        matters.erase(it);
    }
}
