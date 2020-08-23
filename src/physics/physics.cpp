#include "physics/physics.h"

#include "core/scheduler.h"
#include "substance/matter/sdf/compose.h"

#include <chrono>
#include <functional>
#include <iostream>

using namespace std::placeholders;

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
    auto clock_d = std::chrono::duration_cast<scheduler::clock_t::duration>(hyper::iota);
    double physics_d = hyper::iota.count() / 1000.0;

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
    auto error_func = std::bind(collision_error_function, a, b, _1, _2, _3, _4);
}

double
physics_t::collision_error_function(
    std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b,
    double t, 
    double x, double y, double z
){
    return 0.0;
}
