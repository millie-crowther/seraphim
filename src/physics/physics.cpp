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
physics_t::tick(){

}

void 
physics_t::run(){
    auto t = std::chrono::steady_clock::now();
    while (!quit){
        t += std::chrono::microseconds(static_cast<uint32_t>(1000000 * hyper::iota));
        std::this_thread::sleep_until(t);

    }
}