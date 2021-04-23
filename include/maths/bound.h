#ifndef SERAPHIM_BOUND
#define SERAPHIM_BOUND

#include "maths/maths.h"

typedef struct srph_bound3 {
    vec3 lower;
    vec3 upper;
} srph_bound3;

void srph_bound3_create(srph_bound3 * b);
void srph_bound3_intersection(const srph_bound3 * a, const srph_bound3 * b, srph_bound3 * i);
void srph_bound3_vertex(const srph_bound3 * b, int i, vec3 * v);
void srph_bound3_midpoint(const srph_bound3 * b, vec3 * v);
void srph_bound3_radius(const srph_bound3 * b, vec3 * v);
void srph_bound3_capture(srph_bound3 * b, vec3 * v);
bool srph_bound3_contains(srph_bound3 * b, const vec3 * v);

double srph_bound3_volume(const srph_bound3 * b);
void srph_bound3_bisect(const srph_bound3 * self, srph_bound3 * sub_bounds);

#endif
