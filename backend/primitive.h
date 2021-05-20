#ifndef SERAPHIM_PRIMITIVE_H
#define SERAPHIM_PRIMITIVE_H

#include "../common/maths.h"

double sdf_sphere(void *data, const vec3 *x);
double sdf_torus(void *data, const vec3 *x);

#endif
