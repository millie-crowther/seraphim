#ifndef SERAPHIM_PLATONIC_H
#define SERAPHIM_PLATONIC_H

#include "maths/sdf/sdf.h"

double sdf_octahedron(void *data, const vec3 *x);
void sdf_cuboid_create(uint32_t *id, sdf_t *sdf, vec3 *r);

#endif
