#ifndef SERAPHIM_PLATONIC_H
#define SERAPHIM_PLATONIC_H

#include "maths/sdf/sdf.h"

void sdf_cuboid_create(uint32_t *id, sdf_t *sdf, vec3 *r);
void sdf_octahedron_create(uint32_t *id, sdf_t *sdf, double *e);

#endif
