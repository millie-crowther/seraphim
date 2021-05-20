#ifndef SERAPHIM_PHYSICS_H
#define SERAPHIM_PHYSICS_H

#include "metaphysics.h"
#include "collision.h"

#include <thread>
#include <mutex>

typedef struct physics_t {
    int get_frame_count();

    vec3 gravity;

    bool quit;
    std::thread thread;

    std::mutex substances_mutex;

    substance_t *substances;
    uint32_t *num_substances;

    collision_array_t collisions;

    int frames;

    void run();
} physics_t;

void physics_create(physics_t *p, substance_t *substances,
                    uint32_t *num_substances);
void physics_start(physics_t *p);
void physics_destroy(physics_t *p);
void physics_tick(physics_t *p, double dt);

#endif
