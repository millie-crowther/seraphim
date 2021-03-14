#include "physics/physics.h"

#include "core/scheduler.h"

#include <chrono>
#include <functional>
#include <iostream>

void srph_physics_start(srph_physics * p){
    p->quit = false;
    p->thread = std::thread(&srph_physics::run, p);
}

void srph_physics_destroy(srph_physics * p){
    p->quit = true;

    if (p->thread.joinable()){
        p->thread.join();
    }

    printf("joined physics thread\n");
}

void srph_physics_tick(srph_physics * p){
    double t = srph::constant::sigma;

    // update vertices
    srph_matter * m;
    for (uint32_t i = 0; i < p->matters.size(); i++){
        m = p->matters[i];
        srph_matter_update_vertices(m, t);
    }

    // solve constraints

    // update velocities
    for (uint32_t i = 0; i < p->matters.size(); i++){
        m = p->matters[i];
        srph_matter_update_velocities(m, t); 
    }
}

using namespace srph;

void srph_physics::run(){
    auto t = scheduler::clock_t::now();
    auto previous = std::chrono::steady_clock::now();

    printf("physics thread starting\n");
      
    while (!quit){
        frames++;
        
        auto now = std::chrono::steady_clock::now();
        double delta = constant::sigma;

        previous = now;

        std::vector<srph_collision> collisions;
    
        {
            std::lock_guard<std::mutex> lock(matters_mutex);
            
            // reset acceleration and apply gravity force
            for (auto & m : matters){
                if (m->get_position()[1] > -90.0){
                    m->reset_acceleration();
                }
            }

            // collide awake substances with each other
            for (uint32_t i = 0; i < matters.size(); i++){
                for (uint32_t j = i + 1; j < matters.size(); j++){
                    collisions.emplace_back(matters[i], matters[j]);
                }
            }
            
            // collide awake substances with asleep substances
            for (auto awake_matter : matters){
                for (auto asleep_matter : asleep_matters){
                    collisions.emplace_back(asleep_matter, awake_matter);
                }
            }
        }
        
        // correct all present collisions and anticipate the next one
        for (auto & c : collisions){
            if (c.is_intersecting){
                c.correct();
            } 
            delta = fmin(delta, c.t);
        }
        
        delta = std::max(delta, constant::iota);
       
        {
            std::lock_guard<std::mutex> lock(matters_mutex);
 
            // apply acceleration and velocity changes to matters
            for (auto m : matters){
                m->physics_tick(delta);
            } 

            // try to put matters to sleep
            for (uint32_t i = 0; i < matters.size();){
                auto m = matters[i]; 
            
                if (m->is_inert()){
                    std::cout << "Matter going to sleep!" << std::endl;
                    asleep_matters.push_back(m);
                    matters[i] = matters[matters.size() - 1];
                    matters.pop_back();
                } else { 
                    i++;
                }
            }
        }

        t += std::chrono::microseconds(static_cast<int64_t>(delta * 1000000.0));
        std::this_thread::sleep_until(t);
    }
}

void srph_physics::register_matter(srph_matter * matter){
    std::lock_guard<std::mutex> lock(matters_mutex);
    matters.push_back(matter);
}
    
void srph_physics::unregister_matter(srph_matter * matter){
    std::lock_guard<std::mutex> lock(matters_mutex);
    
    auto it = std::find(matters.begin(), matters.end(), matter);
    if (it != matters.end()){
        matters.erase(it);
    } else {
        it = std::find(asleep_matters.begin(), asleep_matters.end(), matter);
        if (it != asleep_matters.end()){
            asleep_matters.erase(it);
        }
    }
}

int srph_physics::get_frame_count(){
    int f = frames;
    frames = 0;
    return f;
}
