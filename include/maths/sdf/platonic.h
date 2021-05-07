#ifndef SERAPHIM_PLATONIC_H
#define SERAPHIM_PLATONIC_H

#include "maths/sdf/sdf.h"

double sdf_cuboid(void *data, const vec3 *x);
double sdf_octahedron(void *data, const vec3 *x);

#endif
