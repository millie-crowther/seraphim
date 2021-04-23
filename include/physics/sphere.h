#ifndef SERAPHIM_SPHERE_H
#define SERAPHIM_SPHERE_H

#include "maths/maths.h"

typedef struct sphere_t {
    vec3 c;
    double r;
} sphere_t;

bool srph_sphere_contains_point(const sphere_t * s, const vec3 * x);
bool srph_sphere_contains(const sphere_t * a, const sphere_t * b);
bool srph_sphere_intersect(const sphere_t * a, const sphere_t * b);
void srph_sphere_capture(sphere_t *a, const sphere_t *b);

#endif
