#ifndef SERAPHIM_COLLISION_H
#define SERAPHIM_COLLISION_H

#include "../common/substance.h"

typedef struct collision_t {
    substance_t *substances[2];
    bound3_t bound;
    array_t(vec3) manifold;
} collision_t;

typedef array_t(collision_t) collision_array_t;

void collision_detect(substance_t *substance_ptrs, size_t num_substances,
                      collision_array_t *cs, double dt);
void collision_resolve(collision_t *self, double dt);

#endif
