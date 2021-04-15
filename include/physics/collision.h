#ifndef SERAPHIM_COLLISION_H
#define SERAPHIM_COLLISION_H

#include "metaphysics/substance.h"

typedef struct srph_collision {
    srph_matter * ms[2];
    vec3 x;
    bool is_colliding;
} srph_collision;

void srph_narrow_phase_collision(srph_collision * c, double dt);
void srph_collision_resolve_interpenetration_constraint(srph_collision * c);
void srph_collision_correct(srph_collision *self, double dt);

#endif
