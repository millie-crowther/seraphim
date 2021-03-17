#ifndef SERAPHIM_PHYSICS_H
#define SERAPHIM_PHYSICS_H

#include "metaphysics/substance.h"

#include <thread>

typedef struct srph_physics {
    int get_frame_count();

    bool quit;
    std::thread thread;

    std::mutex substances_mutex;

    srph_array substances;

    srph_array constraints;

    int frames;

    void run();
} srph_physics;

void srph_physics_init(srph_physics * p);
void srph_physics_start(srph_physics * p);
void srph_physics_destroy(srph_physics *p);
void srph_physics_tick(srph_physics * p);

void srph_physics_register(srph_physics * p, srph_substance * s);
void srph_physics_unregister(srph_physics * p, srph_substance * s);

#endif
