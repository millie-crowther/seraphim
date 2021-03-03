#ifndef SERAPHIM_SPHERE_H
#define SERAPHIM_SPHERE_H

#include "maths/vector.h"

typedef struct srph_sphere {
    vec3 c;
    double r;
} srph_sphere;

bool srph_sphere_contains(const srph_sphere * s, const vec3 * x);
bool srph_sphere_intersect(const srph_sphere * a, const srph_sphere * b);

#endif
