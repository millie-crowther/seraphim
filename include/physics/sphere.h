#ifndef SERAPHIM_SPHERE_H
#define SERAPHIM_SPHERE_H

#include "maths/maths.h"

typedef struct sphere_t {
    vec3 c;
    double r;
} sphere_t;

bool sphere_contains_point(const sphere_t * s, const vec3 * x);
bool sphere_contains(const sphere_t * a, const sphere_t * b);
bool sphere_intersect(const sphere_t * a, const sphere_t * b);
void sphere_capture(sphere_t *a, const sphere_t *b);

#endif
