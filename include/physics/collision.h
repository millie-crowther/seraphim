#ifndef SERAPHIM_COLLISION_H
#define SERAPHIM_COLLISION_H

#include "metaphysics/substance.h"

typedef struct collision_t {
    substance_t *substances[2];
    bound3_t bound;
    array_t(vec3) manifold;
} collision_t;

typedef array_t(collision_t) srph_collision_array;

void collision_detect(substance_t *substance_ptrs, size_t num_substances,
                      srph_collision_array *cs, double dt);
void collision_resolve(collision_t *self, double dt);

#endif
