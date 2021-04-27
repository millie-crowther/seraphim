#ifndef SERAPHIM_SDF_H
#define SERAPHIM_SDF_H

#include "core/array.h"

#include "maths/maths.h"
#include "physics/bound.h"
#include "physics/sphere.h"

#define SERAPHIM_SDF_VOLUME_SAMPLES 10000

typedef double (*sdf_func_t)(void *data, const vec3 * x);

typedef struct sdf_t {
	uint32_t id;

	bool is_bound_valid;
	bound3_t bound;

	double volume;

	bool is_com_valid;
	bool is_inertia_tensor_valid;
	vec3 com;
	mat3 inertia_tensor;

	void *data;
	sdf_func_t distance_function;
} sdf_t;

void sdf_create(sdf_t * sdf, sdf_func_t phi, void *data, uint32_t id);
void sdf_destroy(sdf_t * sdf);

double sdf_distance(sdf_t * sdf, const vec3 * x);
vec3 srph_sdf_normal(sdf_t * sdf, const vec3 * x);
double srph_sdf_volume(sdf_t * sdf);
double srph_sdf_project(sdf_t * sdf, const vec3 * d);
bool srph_sdf_contains(sdf_t * sdf, const vec3 * x);
bound3_t *srph_sdf_bound(sdf_t * sdf);
double srph_sdf_discontinuity(sdf_t * sdf, const vec3 * x);

#endif
