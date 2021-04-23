#include "physics/physics.h"

#include "core/scheduler.h"
#include "physics/collision.h"

#include <chrono>
#include <iostream>

#include <assert.h>

#define SOLVER_ITERATIONS 20

void srph_physics_init(srph_physics *p, srph_substance *substances, size_t *num_substances) {
    p->quit = false;

    p->gravity = {{0.0, -9.8, 0.0}};

    srph_array_init(&p->constraints);
    srph_array_init(&p->collisions);
    p->substances = substances;
    p->num_substances = num_substances;
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
    srph_array_clear(&p->collisions);
}

void srph_physics_tick(srph_physics * p, double dt){
    // update substances and integrate forces
    for (uint32_t i = 0; i < *p->num_substances; i++){
        srph_matter * m = &p->substances[i].matter;

        srph_matter_calculate_sphere_bound(m, dt);

        if (!m->is_at_rest && !m->is_static){
            srph_matter_integrate_forces(m, dt, &p->gravity);
        }
    }

    // broad phase
    srph_broad_phase_collision(p->substances, *p->num_substances, &p->collisions);

    // collision detection
    for (size_t i = 0; i < p->collisions.size; i++){
        srph_narrow_phase_collision(&p->collisions.data[i], dt);
        branch_and_bound_narrow_phase(&p->collisions.data[i]);
    }

    for (int solver_iteration = 0; solver_iteration < SOLVER_ITERATIONS; solver_iteration++){
        for (size_t collision = 0; collision < p->collisions.size; collision++){
            srph_collision * c = &p->collisions.data[collision];
            if (c->is_colliding){
                srph_collision_correct(c, dt);
            }
        }
    }

    // solve constraints
    for (int solver_iteration = 0; solver_iteration < SOLVER_ITERATIONS; solver_iteration++){
        for (size_t collision = 0; collision < p->collisions.size; collision++){
            srph_collision * c = &p->collisions.data[collision];
            if (c->is_colliding) {
                srph_collision_resolve_interpenetration_constraint(c);
            }
        }
    }

    // integrate velocities
    for (uint32_t i = 0; i < *p->num_substances; i++) {
        srph_matter *m = &p->substances[i].matter;

        // integrate linear velocity
        vec3 dv;
        vec3_multiply_f(&dv, &m->v, dt);
        srph_transform_translate(&m->transform, &dv);

        // integrate angular velocity
        vec3 dw;
        vec3_multiply_f(&dw, &m->omega, dt);
        quat q;
        quat_from_euler_angles(&q, &dw);
        assert(isfinite(m->omega.x));
        srph_transform_rotate(&m->transform, &q);

        // dampen velocities
//        const double dampening = 0.99;
//        vec3_multiply_f(&m->v, &m->v, dampening);
//        vec3_multiply_f(&m->omega, &m->omega, dampening);
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

int srph_physics::get_frame_count(){
    int f = frames;
    frames = 0;
    return f;
}
