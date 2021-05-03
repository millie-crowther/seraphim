#include "maths/sdf/primitive.h"

#include <math.h>
#include <stdlib.h>

double sphere_phi(void *data, const vec3 *x) {
    double r = *((double *)data);
    return vec3_length(x) - r;
}

double torus_phi(void *data, const vec3 *x) {
    double *rs = (double *)data;
    vec2 xy;
    xy.x = hypot(x->x, x->z) - rs[0];
    xy.y = x->y;
    return hypot(xy.x, xy.y) - rs[1];
}

void sdf_sphere_create(uint32_t *id, sdf_t *sdf, double *r) {
    sdf_create(id, sdf, sphere_phi, r);
}

void sdf_torus_create(uint32_t *id, sdf_t *sdf, vec2 *rs) {
    sdf_create(id, sdf, torus_phi, rs);
}
