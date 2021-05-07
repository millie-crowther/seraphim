#include "maths/sdf/primitive.h"

#include <math.h>

double sdf_sphere(void *data, const vec3 *x) {
    double r = *((double *)data);
    return vec3_length(x) - r;
}

double sdf_torus(void *data, const vec3 *x) {
    double *rs = (double *)data;
    vec2 xy;
    xy.x = hypot(x->x, x->z) - rs[0];
    xy.y = x->y;
    return hypot(xy.x, xy.y) - rs[1];
}
