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

    uint32_t current_frame = 0;
    uint32_t frequency = 100;
    auto previous = std::chrono::steady_clock::now();

    while (!quit){
        auto now = std::chrono::steady_clock::now();
        double delta = std::chrono::duration_cast<std::chrono::microseconds>(now - previous).count() / 1000000.0;
        previous = now;
        if (current_frame % frequency == frequency - 1){
            std::cout << "Physics FPS: " << 1.0 / delta << std::endl;
        }
        current_frame++;

        std::vector<collision_t> collisions;

        for (auto a_it = matters.begin(); a_it != matters.end(); a_it++){
            for (auto b_it = std::next(a_it); b_it != matters.end(); b_it++){
                
                collision_t c = collide(*a_it, *b_it);
                if (c.hit){
                    collisions.push_back(c);
                }
            }
        }

        for (auto & c : collisions){
            c.correct();
        }
        
        for (auto & m : matters){
            m->physics_tick(delta);
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
