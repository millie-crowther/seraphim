#include "physics/physics.h"

#include "core/scheduler.h"
#include "physics/collision.h"

#include <chrono>
#include <functional>
#include <iostream>

using namespace srph;

physics_t::physics_t(){
    quit = false;
    thread = std::thread(&physics_t::run, this);
}

physics_t::~physics_t(){
    quit = true;
    thread.join();
}

void physics_t::run(){
    auto t = scheduler::clock_t::now();
    auto clock_d = std::chrono::duration_cast<scheduler::clock_t::duration>(constant::iota);

    uint32_t current_frame = 0;
    uint32_t frequency = 100;
    auto previous = std::chrono::steady_clock::now() - clock_d;
    double p_time = 0.0;
       
    while (!quit){
        auto now = std::chrono::steady_clock::now();
        double delta = std::chrono::duration_cast<std::chrono::microseconds>(now - previous).count() / 1000000.0;

        previous = now;

        std::vector<collision_t> intersecting;
        std::vector<collision_t> anticipated;

        for (auto a_it = matters.begin(); a_it != matters.end(); a_it++){
            for (auto b_it = std::next(a_it); b_it != matters.end(); b_it++){
                collision_t c(delta, *a_it, *b_it);
                
                if (c.is_intersecting()){
                    intersecting.push_back(c);
                } else if (c.is_anticipated()){
                    anticipated.push_back(c);
                }
            }
        }

        for (auto & c : intersecting){
            c.correct();
        }

        if (!anticipated.empty()){
            std::sort(anticipated.begin(), anticipated.end(), collision_t::comparator_t());
            delta = anticipated[0].get_estimated_time();
        } 
        
        for (auto & m : matters){
            m->physics_tick(delta);
        } 

        for (auto & m : matters){
            if (m->get_position()[1] > -90.0){
                if (m->is_inert(delta)){
                    std::cout << "inert!" << std::endl;
                }
            } 
        }

        p_time += 1.0 / delta;        
        if (current_frame % frequency == frequency - 1){
            std::cout << "Physics FPS: " << p_time / frequency << std::endl;
            p_time = 0;
        }
        current_frame++;

        t += std::chrono::microseconds(static_cast<int64_t>(delta * 1000000.0));
        std::this_thread::sleep_until(t);
    }
}

void physics_t::register_matter(std::shared_ptr<matter_t> matter){
    matters.insert(matter);
}
    
void physics_t::unregister_matter(std::shared_ptr<matter_t> matter){
    auto it = matters.find(matter);
    if (it != matters.end()){
        matters.erase(it);
    }
}
