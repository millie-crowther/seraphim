#include "physics/physics.h"

physics_t::physics_t(){
    quit = false;

    thread = std::thread(&physics_t::run, this);
}

physics_t::~physics_t(){
    quit = true;
    thread.join();
}

void
physics_t::tick(double delta){

}

void 
physics_t::run(){
    while (!quit){
        tick(0);
    }
}