#ifndef SERAPHIM_COLLISION_H
#define SERAPHIM_COLLISION_H

#include "metaphysics/substance.h"

#define COLLISION_MANIFOLD_MAX_SIZE 3

typedef struct collision_t {
    srph_matter * ms[2];
    bound3_t bound;
    vec3 manifold[COLLISION_MANIFOLD_MAX_SIZE];
    size_t manifold_size;
} collision_t;

typedef srph_array(collision_t) srph_collision_array;

void collision_detect(srph_substance *substance_ptrs, size_t num_substances, srph_collision_array *cs, double dt);
void collision_resolve_interpenetration_constraint(collision_t * c);
void collision_resolve_velocity_constraint(collision_t *self, double dt);

#endif
