#include "physics/physics.h"

#include "core/scheduler.h"
#include "physics/collision.h"

#include <assert.h>

#include <chrono>
#include <iostream>

#define SOLVER_ITERATIONS 10

void srph_physics_init(srph_physics * p){
    p->quit = false;

    p->gravity = { {{0.0, -9.8, 0.0}} };

    srph_array_init(&p->substances);
    srph_array_init(&p->constraints);
    srph_array_init(&p->collisions);
} 

void srph_physics_start(srph_physics * p){
    p->thread = std::thread(&srph_physics::run, p);
}

void srph_physics_destroy(srph_physics * p){
    p->quit = true;

    if (p->thread.joinable()){
        p->thread.join();
    }

    srph_array_clear(&p->constraints);
    srph_array_clear(&p->substances);
    srph_array_clear(&p->collisions);
}

void srph_physics_tick(srph_physics * p, double dt){
    srph_array_clear(&p->collisions);

    // update vertices 
    for (uint32_t i = 0; i < p->substances.size; i++){
        srph_matter * m = &p->substances.data[i]->matter;

        if (!m->is_at_rest && !m->is_static){
            srph_matter_integrate_forces(m, dt, &p->gravity);
        }
    }

    // collision detection
    for (uint32_t i = 0; i < p->substances.size; i++){
        srph_substance * s = p->substances.data[i];

        for (uint32_t j = i + 1; j < p->substances.size; j++){
            srph_substance * t = p->substances.data[j];
            srph_collision c;
            if (srph_collision_is_detected(&c, s, t, dt)){
                srph_array_push_back(&p->collisions);
                *p->collisions.last = c;
            }
        }
    }

    // solve constraints
    for (int i = 0; i < SOLVER_ITERATIONS; i++){
        for (size_t j = 0; j < p->collisions.size; j++){
            srph_collision_resolve_interpenetration_constraint(&p->collisions.data[j]);
        }
    }

    // collision contact correct
    for (size_t i = 0; i < p->collisions.size; i++){
        srph_collision_correct(&p->collisions.data[i]);
    }
}

using namespace srph;

void srph_physics::run(){
    auto t = scheduler::clock_t::now();
    printf("physics thread starting\n");
      
    while (!quit){
        frames++;
        
        double delta = constant::sigma;

        srph_physics_tick(this, delta);
/*
        std::vector<srph_collision> collisions;
    
        {
            std::lock_guard<std::mutex> lock(substances_mutex);
            
            // reset acceleration and apply gravity force
            for (uint32_t i = 0; i < substances.size; i++){
                srph_substance * s = substances.data[i];
                
                if (s->matter.get_position()[1] > -90.0){
                    s->matter.reset_acceleration();
                }
            }

            // collide awake substances with each other
            for (uint32_t i = 0; i < substances.size; i++){
                srph_substance * s = substances.data[i];
               
                for (uint32_t j = i + 1; j < substances.size; j++){
                    srph_substance * t = substances.data[j];
                    collisions.emplace_back(&s->matter, &t->matter);
                }
            }
        }
       
        {
            std::lock_guard<std::mutex> lock(substances_mutex);

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

        */
        t += std::chrono::microseconds(static_cast<int64_t>(delta * 1000000.0));
        std::this_thread::sleep_until(t);
    }
}

void srph_physics_register(srph_physics * p, srph_substance * s){
    std::lock_guard<std::mutex> lock(p->substances_mutex);
    srph_array_push_back(&p->substances);
    *p->substances.last = s;
}
    
void srph_physics_unregister(srph_physics * p, srph_substance * s){
    std::lock_guard<std::mutex> lock(p->substances_mutex);
    
    for (uint32_t i = 0; i < p->substances.size;){
        srph_substance * t = p->substances.data[i];
        if (s == t){
            p->substances.data[i] = *p->substances.last;
            srph_array_pop_back(&p->substances);
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
