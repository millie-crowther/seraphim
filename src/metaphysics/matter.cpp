#include "metaphysics/matter.h"

#include <assert.h>
#include <math.h>

#include "core/random.h"

#define LINEAR_VELOCITY_REST_THRESHOLD 0.075
#define ANGULAR_VELOCITY_REST_THRESHOLD 0.2

void
srph_matter_init(matter_t * m, sdf_t * sdf,
	const material_t * material,
	const vec3 * initial_position, bool is_uniform, bool is_static) {
	m->sdf = sdf;
	m->material = *material;
	m->is_uniform = is_uniform;
	m->is_static = is_static;
	m->is_rigid = true;
	m->is_at_rest = false;
	m->has_collided = false;
	m->transform.position = initial_position == NULL ? vec3_zero : *initial_position;
	m->transform.rotation = quat_identity;
	m->v = vec3_zero;
	m->omega = vec3_zero;
	m->f = vec3_zero;
	m->t = vec3_zero;

	if (m->transform.position.y > -90) {
		m->omega = { {0.1, 0.1, 0.1}
		};
	}

	srph_array_init(&m->deformations);
}

void srph_matter_destroy(matter_t * m) {
	while (!srph_array_is_empty(&m->deformations)) {
		free(*m->deformations.last);
		srph_array_pop_back(&m->deformations);
	}
	srph_array_clear(&m->deformations);
}

bool matter_is_at_rest(matter_t * m) {
	double v = vec3_length(&m->v);
	double w = vec3_length(&m->omega);

	return
		m->has_collided &&
		v <= LINEAR_VELOCITY_REST_THRESHOLD && w <= ANGULAR_VELOCITY_REST_THRESHOLD;
}

double srph_matter_average_density(matter_t * self) {
	if (self->is_uniform) {
		return self->material.density;
	}

	assert(false);

	return 0.0;
}

double srph_matter_mass(matter_t * self) {
	return srph_matter_average_density(self) * srph_sdf_volume(self->sdf);
}

void srph_matter_inverse_inertia_tensor(matter_t * self, mat3 * ri) {
	if (self->is_static) {
		return;
	}

	mat3 *i = srph_matter_inertia_tensor(self);

	mat3 r, rt;
	mat3_rotation_quat(&r, &self->transform.rotation);
	mat3_transpose(&rt, &r);

	mat3_multiply(ri, i, &rt);
	mat3_multiply(ri, &r, ri);
	mat3_inverse(ri, ri);
}

void srph_matter_calculate_sphere_bound(matter_t * self, double dt) {
	vec3 midpoint, radius;
	srph_bound3_midpoint(&self->sdf->bound, &midpoint);
	srph_bound3_radius(&self->sdf->bound, &radius);
	srph_matter_to_global_position(self, &self->bounding_sphere.c, &midpoint);
	self->bounding_sphere.r = vec3_length(&radius) + vec3_length(&self->v) * dt;
}

srph_deform *srph_matter_add_deformation(matter_t * self, const vec3 * x,
	srph_deform_type type) {
	// transform position into local space
	vec3 x0;
	srph_matter_to_local_position(self, &x0, x);

	// check if new deformation is inside surface and not too close to any other deformations
	if (type == srph_deform_type_collision) {
		if (!srph_sdf_contains(self->sdf, &x0)) {
			return NULL;
		}

		for (size_t i = 0; i < self->deformations.size; i++) {
			srph_deform *deform = self->deformations.data[i];
			if (vec3_distance(&x0, &deform->x0) < SERAPHIM_DEFORM_MAX_SAMPLE_DENSITY) {
				return NULL;
			}
		}
	}
	// create new deformation
	srph_deform *deform = (srph_deform *) malloc(sizeof(srph_deform));
	*deform = {
		.x0 = x0,
		.x = *x,
		.v = vec3_zero,
		.p = *x,
		.m = 1.0,				// TODO
		.type = type,
	};

	// find average velocity
	for (size_t i = 0; i < self->deformations.size; i++) {
		vec3_add(&deform->v, &deform->v, &self->deformations.data[i]->v);
	}

	if (!srph_array_is_empty(&self->deformations)) {
		vec3_multiply_f(&deform->v, &deform->v,
			1.0 / (double) self->deformations.size);
	}
	// add to list of deformations
	srph_array_push_back(&self->deformations);
	*self->deformations.last = deform;

	return deform;
}

void srph_matter_to_local_position(matter_t * m, vec3 * tx, const vec3 * x) {
	srph_transform_to_local_position(&m->transform, tx, x);
}

void srph_matter_transformation_matrix(matter_t * m, float *xs) {
	mat4 dxs;
	srph_transform_matrix(&m->transform, &dxs);

	for (int i = 0; i < 16; i++) {
		xs[i] = (float) dxs.v[i];
	}
}

void srph_matter_to_global_position(const matter_t * m, vec3 * tx, const vec3 * x) {
	srph_transform_to_global_position(&m->transform, tx, x);
}

void
srph_matter_to_global_direction(const matter_t * m,
	const vec3 * position, vec3 * td, const vec3 * d) {
	srph_transform_to_global_direction(&m->transform, td, d);
}

void srph_matter_integrate_forces(matter_t * self, double t, const vec3 * gravity) {
	assert(!self->is_static && !self->is_at_rest);

	double m = srph_matter_mass(self);

	// integrate force
	vec3 d;
	vec3_multiply_f(&d, &self->f, t / m);
	vec3_add(&self->v, &self->v, &d);

	// integrate torque
	vec3_multiply_f(&d, &self->t, t / m);
	vec3_add(&self->omega, &self->omega, &d);

	// reset forces
	vec3_multiply_f(&self->f, gravity, m);
	self->t = vec3_zero;
}

static void offset_from_centre_of_mass(matter_t * self, vec3 * r, const vec3 * x) {
	vec3 com;
	srph_matter_to_global_position(self, &com, srph_matter_com(self));
	vec3_subtract(r, x, &com);
}

void srph_matter_velocity(matter_t * self, const vec3 * x, vec3 * v) {
	if (self->is_rigid) {
		vec3 r;
		offset_from_centre_of_mass(self, &r, x);
		vec3_cross(v, &self->omega, &r);
		vec3_add(v, v, &self->v);
	} else {
		assert(false);
	}
}

void srph_matter_material(matter_t * self, material_t * mat, const vec3 * x) {
	// TODO: sample at point
	*mat = self->material;
}

double srph_matter_inverse_angular_mass(matter_t * self, vec3 * x, vec3 * n) {
	if (self->is_static) {
		return 0;
	}

	vec3 r, rn, irn;
	offset_from_centre_of_mass(self, &r, x);
	vec3_cross(&rn, &r, n);
	mat3 i;
	srph_matter_inverse_inertia_tensor(self, &i);
	vec3_multiply_mat3(&irn, &rn, &i);

	return vec3_dot(&rn, &irn);
}

double srph_matter_inverse_mass(matter_t * self) {
	if (self->is_static) {
		return 0;
	} else {
		return 1.0 / srph_matter_mass(self);
	}
}

mat3 *srph_matter_inertia_tensor(matter_t * matter) {
	if (!matter->is_inertia_tensor_valid) {
		if (matter->is_uniform && matter->sdf->is_inertia_tensor_valid) {
			matter->inertia_tensor = matter->sdf->inertia_tensor;
		} else {
			for (int i = 0; i < 9; i++) {
				matter->inertia_tensor.v[i] = 0.0;
			}

			bound3_t *b = srph_sdf_bound(matter->sdf);
			random_t rng;
			srph_random_default_seed(&rng);
			int hits = 0;
			double total = 0.0;
			material_t mat;
			srph_matter_material(matter, &mat, NULL);

			while (hits < SERAPHIM_SDF_VOLUME_SAMPLES) {
				vec3 x;
				x.x = srph_random_f64_range(&rng, b->lower.x, b->upper.x);
				x.y = srph_random_f64_range(&rng, b->lower.y, b->upper.y);
				x.z = srph_random_f64_range(&rng, b->lower.z, b->upper.z);

				if (!matter->is_uniform) {
					srph_matter_material(matter, &mat, NULL);
				}

				if (srph_sdf_contains(matter->sdf, &x)) {
					for (int i = 0; i < 3; i++) {
						for (int j = 0; j < 3; j++) {
							vec3 r;
							vec3_subtract(&r, &x, srph_matter_com(matter));

							double iij = -r.v[i] * r.v[j];

							if (i == j) {
								iij += vec3_length_squared(&r);
							}

							matter->inertia_tensor.v[j * 3 + i]
								+= iij * mat.density;
						}
					}

					hits++;
					total += mat.density;
				}
			}

			mat3_multiply_f(&matter->inertia_tensor,
				&matter->inertia_tensor, 1.0 / total);

			if (matter->is_uniform && !matter->sdf->is_inertia_tensor_valid) {
				matter->sdf->inertia_tensor = matter->inertia_tensor;
				matter->sdf->is_inertia_tensor_valid = true;
			}
		}

		mat3_multiply_f(&matter->inertia_tensor,
			&matter->inertia_tensor, srph_matter_mass(matter));
		matter->is_inertia_tensor_valid = true;
	}

	return &matter->inertia_tensor;
}

vec3 *srph_matter_com(matter_t * matter) {
	if (matter->is_uniform && matter->sdf->is_com_valid) {
		return &matter->sdf->com;
	}

	if (!matter->is_com_valid) {
		vec3 com = vec3_zero;

		bound3_t *b = srph_sdf_bound(matter->sdf);
		random_t rng;
		srph_random_default_seed(&rng);
		int hits = 0;
		double total = 0.0;
		material_t mat;
		srph_matter_material(matter, &mat, NULL);

		while (hits < SERAPHIM_SDF_VOLUME_SAMPLES) {
			vec3 x;
			x.x = srph_random_f64_range(&rng, b->lower.x, b->upper.x);
			x.y = srph_random_f64_range(&rng, b->lower.y, b->upper.y);
			x.z = srph_random_f64_range(&rng, b->lower.z, b->upper.z);

			if (!matter->is_uniform) {
				srph_matter_material(matter, &mat, NULL);
			}
			vec3_multiply_f(&x, &x, mat.density);

			if (srph_sdf_contains(matter->sdf, &x)) {
				vec3_add(&com, &com, &x);
				hits++;
				total += mat.density;
			}
		}

		vec3_divide_f(&com, &com, total);
		matter->com = com;
		matter->is_com_valid = true;

		if (matter->is_uniform && !matter->sdf->is_com_valid) {
			matter->sdf->com = com;
			matter->sdf->is_com_valid = true;
		}
	}

	return &matter->com;
}

void apply_impulse(matter_t * self, const vec3 * x, const vec3 * j) {
	vec3 n;
	vec3_normalize(&n, j);
	double j_length = vec3_length(j);

	if (self->is_static || j_length == 0) {
		return;
	}

	self->is_at_rest = false;

	vec3 dv;
	vec3_multiply_f(&dv, &n, j_length * srph_matter_inverse_mass(self));
	vec3_add(&self->v, &self->v, &dv);

	vec3 r, rn, dw;
	offset_from_centre_of_mass(self, &r, x);
	vec3_cross(&rn, &r, &n);

	mat3 i;
	vec3 irn;
	srph_matter_inverse_inertia_tensor(self, &i);
	vec3_multiply_mat3(&irn, &rn, &i);
	vec3_multiply_f(&dw, &irn, j_length);
	vec3_add(&self->omega, &self->omega, &dw);
}

void matter_apply_impulse(matter_t * a, matter_t * b, const vec3 * x, const vec3 * j) {
	vec3 j_negative;
	vec3_negative(&j_negative, j);

	apply_impulse(a, x, j);
	apply_impulse(b, x, &j_negative);
}
