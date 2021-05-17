#include "physics/physics.h"

#include "core/constant.h"
#include "physics/collision.h"

#include <chrono>
#include <iostream>

#include <assert.h>

#define SOLVER_ITERATIONS 1

void physics_create(physics_t *p, substance_t *substances,
                    uint32_t *num_substances) {
    p->quit = false;

    p->gravity = {{0.0, -9.8, 0.0}};

    array_create(&p->collisions);
    p->substances = substances;
    p->num_substances = num_substances;
}

void physics_start(physics_t *p) {
    p->thread = std::thread(&physics_t::run, p);
}

void physics_destroy(physics_t *p) {
    p->quit = true;

    if (p->thread.joinable()) {
        p->thread.join();
    }

    array_clear(&p->collisions);
}

void physics_tick(physics_t *p, double dt) {
    // update substances and integrate forces
    for (uint32_t i = 0; i < *p->num_substances; i++) {
        substance_t *substance = &p->substances[i];
        substance->matter.has_collided = false;

        substance_calculate_sphere_bound(substance, dt);

        if (!substance->matter.is_at_rest && !substance->matter.is_static) {
            matter_integrate_forces(&substance->matter, dt, &p->gravity,
                                    substance_mass(substance));
        }
    }

    // detect collisions
    collision_detect(p->substances, *p->num_substances, &p->collisions, dt);

    // resolve collisions
    for (int solver_iteration = 0; solver_iteration < SOLVER_ITERATIONS;
         solver_iteration++) {
        for (size_t collision = 0; collision < p->collisions.size; collision++) {
            collision_resolve(&p->collisions.data[collision], dt);
        }
    }

    // integrate velocities
    for (uint32_t i = 0; i < *p->num_substances; i++) {
        matter_t *m = &p->substances[i].matter;

        if (m->is_static || m->is_at_rest) {
            continue;
        }
        // integrate linear velocity
        vec3 dv;
        vec3_multiply_f(&dv, &m->velocity, dt);
        transform_translate(&m->transform, &dv);

        // integrate angular velocity
        vec3 dw;
        vec3_multiply_f(&dw, &m->angular_velocity, dt);
        quat q;
        quat_from_euler_angles(&q, &dw);
        transform_rotate(&m->transform, &q);
    }

    // attempt to put substances to sleep
    for (size_t i = 0; i < *p->num_substances; i++) {
        matter_t *m = &p->substances[i].matter;
        if (matter_is_at_rest(m)) {
            m->is_at_rest = true;
        }
    }
}

void physics_t::run() {
    auto t = std::chrono::steady_clock::now();
    printf("physics thread starting\n");

    while (!quit) {
        frames++;
        double delta = sigma;
        physics_tick(this, delta);
        t += std::chrono::microseconds(static_cast<int64_t>(delta * 1000000.0));
        std::this_thread::sleep_until(t);
    }
}

int physics_t::get_frame_count() {
    int f = frames;
    frames = 0;
    return f;
}
