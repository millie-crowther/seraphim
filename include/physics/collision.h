#ifndef SERAPHIM_COLLISION_H
#define SERAPHIM_COLLISION_H

#include "metaphysics/substance.h"

typedef struct collision_t {
    srph_matter * ms[2];
    bool is_colliding;
    srph_bound3 bound;
} collision_t;

void srph_collision_resolve_interpenetration_constraint(collision_t * c);
void srph_collision_correct(collision_t *self, double dt);
bool collision_narrow_phase_branch_and_bound(collision_t *c);
void collision_generate_manifold(collision_t * c, double dt);

#endif
