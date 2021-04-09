#ifndef SERAPHIM_SDF_H
#define SERAPHIM_SDF_H

#include "core/array.h"

#include "maths/maths.h"
#include "maths/bound.h"

typedef double (*srph_sdf_func)(void * data, const vec3 * x);

typedef struct srph_sdf {
    bool is_bound_valid;
    srph_bound3 bound;

    double volume;

    bool is_com_valid;
    vec3 com;

    bool is_inertia_tensor_valid;
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
srph_bound3 * srph_sdf_bound(srph_sdf * sdf);
vec3 * srph_sdf_com(srph_sdf * sdf);

mat3 * srph_sdf_inertia_tensor(srph_sdf * sdf);

#endif
