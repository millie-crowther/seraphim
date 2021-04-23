#ifndef SERAPHIM_SDF_H
#define SERAPHIM_SDF_H

#include "core/array.h"

#include "maths/maths.h"
#include "physics/bound.h"
#include "physics/sphere.h"

#define SERAPHIM_SDF_VOLUME_SAMPLES 10000

typedef double (*srph_sdf_func)(void * data, const vec3 * x);

typedef struct srph_sdf {
    bool is_bound_valid;
    bound3_t bound;

    double volume;

    bool is_com_valid;
    bool is_inertia_tensor_valid;
    vec3 com;
    mat3 inertia_tensor;

    void * data;
    srph_sdf_func _phi;
} srph_sdf;

void srph_sdf_create(srph_sdf * sdf, srph_sdf_func phi, void * data);
void srph_sdf_destroy(srph_sdf * sdf);

double srph_sdf_phi(srph_sdf * sdf, const vec3 * x);
vec3 srph_sdf_normal(srph_sdf * sdf, const vec3 * x);
double srph_sdf_volume(srph_sdf * sdf);
double srph_sdf_project(srph_sdf * sdf, const vec3 * d);
bool srph_sdf_contains(srph_sdf * sdf, const vec3 * x);
bound3_t * srph_sdf_bound(srph_sdf * sdf);
double srph_sdf_discontinuity(srph_sdf *sdf, const vec3 *x);

#endif
