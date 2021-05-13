#ifndef SERAPHIM_PHYSICS_H
#define SERAPHIM_PHYSICS_H

#include "metaphysics/substance.h"
#include "physics/collision.h"

#include <thread>
#include <mutex>

typedef struct srph_physics {
    int get_frame_count();

    vec3 gravity;

    bool quit;
    std::thread thread;

    std::mutex substances_mutex;

    substance_t *substances;
    uint32_t *num_substances;

    srph_collision_array collisions;

    int frames;

    void run();
} srph_physics;

void physics_create(srph_physics *p, substance_t *substances,
                    uint32_t *num_substances);
void physics_start(srph_physics *p);
void physics_destroy(srph_physics *p);
void physics_tick(srph_physics *p, double dt);

#endif
