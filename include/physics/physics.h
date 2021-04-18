#ifndef SERAPHIM_PHYSICS_H
#define SERAPHIM_PHYSICS_H

#include "metaphysics/substance.h"
#include "physics/broad_phase.h"
#include "physics/collision.h"

#include <thread>

typedef struct srph_physics {
    int get_frame_count();

    vec3 gravity;

    bool quit;
    std::thread thread;

    std::mutex substances_mutex;

    srph_substance * substances;
    size_t * num_substances;

    srph_constraint_array constraints;
    srph_collision_array collisions;

    int frames;

    void run();
} srph_physics;

void srph_physics_init(srph_physics *p, srph_substance *substances, size_t *num_substances);
void srph_physics_start(srph_physics * p);
void srph_physics_destroy(srph_physics *p);
void srph_physics_tick(srph_physics * p, double dt);

#endif
