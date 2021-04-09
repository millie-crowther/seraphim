#ifndef SERAPHIM_SPHERE_H
#define SERAPHIM_SPHERE_H

#include "maths/maths.h"

typedef struct srph_sphere {
    vec3 c;
    double r;
} srph_sphere;

bool srph_sphere_contains(const srph_sphere * a, const srph_sphere * b);
bool srph_sphere_intersect(const srph_sphere * a, const srph_sphere * b);

#endif
