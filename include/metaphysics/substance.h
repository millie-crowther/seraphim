#ifndef SERAPHIM_SUBSTANCE_H
#define SERAPHIM_SUBSTANCE_H

#include <memory>

#include "form.h"
#include "matter.h"

#include "maths/matrix.h"

struct data_t {
	float near;
	float far;
	uint32_t sdf_id;
	float material_id;

	  srph::f32vec3_t r;
	uint32_t id;

	float transform[16];

	  data_t();
	  data_t(float near, float far, const srph::f32vec3_t & r, uint32_t id);

	struct comparator_t {
		bool operator          () (const data_t & a, const data_t & b) const;
	};
};

typedef struct substance_t {
	substance_t();
	substance_t(form_t * form, matter_t * matter, uint32_t i);

	data_t get_data(const vec3 * eye_position);

	uint32_t id;
	form_t form;
	matter_t matter;
} substance_t;

// velocity
void substance_velocity_at(substance_t * self, const vec3 * x, vec3 * v);
void substance_apply_impulse(substance_t * a, substance_t * b, const vec3 * x,
	const vec3 * j);

// angular mass
double substance_inverse_angular_mass(substance_t * self, vec3 * x, vec3 * n);
mat3 *substance_inertia_tensor(substance_t * self);
void substance_inverse_inertia_tensor(substance_t * self, mat3 * ri);

// mass
double substance_mass(substance_t * self);
double substance_inverse_mass(substance_t * self);
vec3 *substance_com(substance_t * self);

#endif
