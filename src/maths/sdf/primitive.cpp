#include "maths/sdf/primitive.h"

#include <math.h>
#include <stdlib.h>

double sphere_phi(void *data, const vec3 * x) {
	double r = *((double *) data);
	return vec3_length(x) - r;
}

double torus_phi(void *data, const vec3 * x) {
	double *rs = (double *) data;
	vec2 xy;
	xy.x = hypot(x->x, x->z) - rs[0];
	xy.y = x->y;
	return hypot(xy.x, xy.y) - rs[1];
}

sdf_t *sdf_sphere_create(seraphim_t * seraphim, double r) {
	double *r2 = (double *) malloc(sizeof(double));
	*r2 = r;
	return seraphim_create_sdf(seraphim, sphere_phi, r2);
}

sdf_t *sdf_torus_create(seraphim_t * seraphim, double r1, double r2) {
	double *rs = (double *) malloc(2 * sizeof(double));
	rs[0] = r1;
	rs[1] = r2;
	return seraphim_create_sdf(seraphim, torus_phi, rs);
}
