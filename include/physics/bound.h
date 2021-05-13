#ifndef SERAPHIM_BOUND
#define SERAPHIM_BOUND

#include "maths/maths.h"

typedef struct bound3_t {
    vec3 lower;
    vec3 upper;
} bound3_t;

void bound3_create(bound3_t *b);
void bound3_intersection(const bound3_t *a, const bound3_t *b, bound3_t *intersection);
void bound3_vertex(const bound3_t *b, int i, vec3 *v);
void bound3_midpoint(const bound3_t *b, vec3 *v);
void bound3_radius(const bound3_t *b, vec3 *v);
void bound3_capture(bound3_t *b, vec3 *v);
bool bound3_contains(bound3_t *b, const vec3 *v);
bool bound3_is_valid(bound3_t *b);

double bound3_volume(const bound3_t *b);
void bound3_bisect(const bound3_t *self, bound3_t *sub_bounds);

#endif
