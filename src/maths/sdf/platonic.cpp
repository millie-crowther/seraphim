#include "maths/sdf/platonic.h"

#include <stdlib.h>

static double cuboid_phi(void *data, const vec3 * x)
{
	vec3 *r = (vec3 *) data;
	vec3 x1 = *x;

	vec3_abs(&x1, &x1);

	vec3 q;
	vec3_subtract(&q, &x1, r);

	double m = q.x;
	for (int i = 0; i < 3; i++) {
		m = fmax(m, q.v[i]);
	}

	for (int i = 0; i < 3; i++) {
		q.v[i] = fmax(q.v[i], 0.0);
	}

	return vec3_length(&q) + fmin(m, 0.0);
}

static double octahedron_phi(void *data, const vec3 * x)
{
	double e = *((double *)data);
	double s = e / sqrt(2);
	vec3 p = *x;
	vec3_abs(&p, &p);

	float m = p.x + p.y + p.z - s;

	vec3 q;
	if (3.0 * p.x < m) {
		q = { {p.x, p.y, p.z}
		};
	} else if (3.0 * p.y < m) {
		q = { {p.y, p.z, p.x}
		};
	} else if (3.0 * p.z < m) {
		q = { {p.z, p.x, p.y}
		};
	} else {
		return m * 0.57735027;
	}

	float k = 0.5 * (q.z - q.y + s);
	k = fmax(k, 0.0);
	k = fmin(k, s);

	vec3 r = { {q.x, q.y - s + k, q.z - k} };
	return vec3_length(&r);
}

sdf_t *sdf_cuboid_initialise(srph::seraphim_t * seraphim, const vec3 * r)
{
	vec3 *r_ptr = (vec3 *) malloc(sizeof(vec3));
	*r_ptr = *r;

	sdf_t *sdf = seraphim_create_sdf(seraphim, cuboid_phi, r_ptr);

	vec3 inertia;
	vec3_multiply_f(&inertia, r, 2.0);
	vec3_multiply(&inertia, &inertia, &inertia);
	vec3_multiply_f(&inertia, &inertia, 1.0 / 6.0);

	for (int i = 0; i < 9; i++) {
		sdf->inertia_tensor.v[i] = 0.0;
	}

	mat3_scale(&sdf->inertia_tensor, &mat3_identity, &inertia);
	sdf->is_inertia_tensor_valid = true;

	sdf->com = vec3_zero;
	sdf->is_com_valid = true;
	return sdf;
}

sdf_t *sdf_octahedron_initialise(srph::seraphim_t * seraphim, double e)
{
	double *e2 = (double *)malloc(sizeof(double));
	*e2 = e;
	return seraphim_create_sdf(seraphim, octahedron_phi, e2);
}
