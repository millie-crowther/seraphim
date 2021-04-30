#ifndef SERAPHIM_MATTER_H
#define SERAPHIM_MATTER_H

#include "material.h"

#include "maths/sdf/sdf.h"
#include "physics/deform.h"
#include "physics/sphere.h"
#include "physics/transform.h"

typedef struct matter_t {
	transform_t transform;
	vec3 v;
	vec3 omega;

	material_t material;
	sdf_t *sdf;

	  srph_array(srph_deform *) deformations;

	bool is_uniform;
	bool is_at_rest;
	bool is_rigid;
	bool is_static;

	bool is_inertia_tensor_valid;
	bool is_com_valid;
	bool has_collided;
	mat3 inertia_tensor;
	vec3 com;

	mat3 inverse_inertia_tensor;

	sphere_t bounding_sphere;

	vec3 f;
	vec3 t;
} matter_t;

void srph_matter_init(matter_t * m, sdf_t * sdf, const material_t * mat,
	const vec3 * x, bool is_uniform, bool is_static);

void srph_matter_destroy(matter_t * m);

void srph_matter_calculate_sphere_bound(matter_t * self, double dt);

void srph_matter_to_global_position(const matter_t * m, vec3 * tx, const vec3 * x);
void srph_matter_to_local_position(matter_t * m, vec3 * tx, const vec3 * x);

void srph_matter_to_global_direction(const matter_t * m,
	const vec3 * position, vec3 * td, const vec3 * d);

srph_deform *srph_matter_add_deformation(matter_t * self, const vec3 * x,
	srph_deform_type type);

void srph_matter_transformation_matrix(matter_t * m, float *xs);

bool matter_is_at_rest(matter_t * m);

void matter_integrate_forces(matter_t *self, double t, const vec3 *gravity, double mass);

void srph_matter_material(matter_t * self, material_t * mat, const vec3 * x);

double srph_matter_average_density(matter_t * self);

#endif
