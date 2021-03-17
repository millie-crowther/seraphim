#include "physics/physics.h"

#include "core/scheduler.h"
#include "physics/collision.h"

#include <chrono>
#include <iostream>

#define SOLVER_ITERATIONS 10

void srph_physics_init(srph_physics * p){
    p->quit = false;
    srph_array_create(&p->substances, sizeof(srph_substance *));
    srph_array_create(&p->constraints, sizeof(srph_constraint));
} 

void srph_physics_start(srph_physics * p){
    p->thread = std::thread(&srph_physics::run, p);
}

void srph_physics_destroy(srph_physics * p){
    p->quit = true;

    if (p->thread.joinable()){
        p->thread.join();
    }

    srph_array_destroy(&p->constraints);
    srph_array_destroy(&p->substances);

    printf("joined physics thread\n");
}

void srph_physics_tick(srph_physics * p){
    double dt = srph::constant::sigma;

    // update vertices 
    srph_substance * s, * t;
    for (uint32_t i = 0; i < p->substances.size; i++){
        s = * (srph_substance **) srph_array_at(&p->substances, i);
        srph_matter_update_vertices(&s->matter, dt);
    }    

    // get constraints
    for (uint32_t i = 0; i < p->substances.size; i++){
        s = * (srph_substance **) srph_array_at(&p->substances, i);
    
        // internal constraints
        srph_matter_push_internal_constraints(&s->matter, &p->constraints);

        // collision constraints
        if (s->matter.is_at_rest){
            continue;
        }

        for (uint32_t j = i + 1; j < p->substances.size; j++){
            t = * (srph_substance **) srph_array_at(&p->substances, j);
            if (srph_collision_is_detected(s, t, dt)){
                srph_collision_push_constraints(&p->constraints, s, t);
            }
        }
    }    

    // solve constraints
    for (int i = 0; i < SOLVER_ITERATIONS; i++){
         
    }

    // update velocities
    for (uint32_t i = 0; i < p->substances.size; i++){
        s = * (srph_substance **) srph_array_at(&p->substances, i);
        srph_matter_update_velocities(&s->matter, dt);
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
            std::lock_guard<std::mutex> lock(substances_mutex);
            
            // reset acceleration and apply gravity force
            for (uint32_t i = 0; i < substances.size; i++){
                srph_substance * s = *(srph_substance **) srph_array_at(&substances, i);
                
                if (s->matter.get_position()[1] > -90.0){
                    s->matter.reset_acceleration();
                }
            }

            // collide awake substances with each other
            for (uint32_t i = 0; i < substances.size; i++){
                srph_substance * s = *(srph_substance **) srph_array_at(&substances, i);
               
                for (uint32_t j = i + 1; j < substances.size; j++){
                    srph_substance * t = *(srph_substance **) srph_array_at(&substances, j);
                    
                    collisions.emplace_back(&s->matter, &t->matter);
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
            std::lock_guard<std::mutex> lock(substances_mutex);
 
            // apply acceleration and velocity changes to matters
            /*
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
            }*/
        }

        t += std::chrono::microseconds(static_cast<int64_t>(delta * 1000000.0));
        std::this_thread::sleep_until(t);
    }
}

void srph_physics_register(srph_physics * p, srph_substance * s){
    std::lock_guard<std::mutex> lock(p->substances_mutex);
    *((srph_substance **) srph_array_push_back(&p->substances)) = s;
}
    
void srph_physics_unregister(srph_physics * p, srph_substance * s){
    std::lock_guard<std::mutex> lock(p->substances_mutex);
    
    for (uint32_t i = 0; i < p->substances.size;){
        srph_substance ** t = (srph_substance **) srph_array_at(&p->substances, i);
        if (s == *t){
            srph_array_pop_back(&p->substances, *t);
        } else {       
            i++;
        }
    }
}

int srph_physics::get_frame_count(){
    int f = frames;
    frames = 0;
    return f;
}
