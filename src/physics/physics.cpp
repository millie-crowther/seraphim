#include "physics/physics.h"

#include <chrono>
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
    auto t = std::chrono::steady_clock::now();
    while (!quit){
        for (auto & m : matters){
            m->physics_tick(1.0 / hyper::iota);
        }

        t += std::chrono::microseconds(static_cast<uint32_t>(1000000.0 / hyper::iota));
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