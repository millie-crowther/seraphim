#ifndef SERAPHIM_SDF_H
#define SERAPHIM_SDF_H

#include "core/constant.h"
#include "maths/vector.h"
#include "maths/bound.h"
#include "maths/matrix.h"

typedef double (*srph_sdf_func)(void * data, const vec3 * x);

typedef struct srph_sdf {
    bool _is_bound_valid;
    srph_bound3 _bound;

    double _volume;

    bool _is_com_valid;
    vec3 _com;

    bool _is_inertia_tensor_valid;
    srph::mat3_t _inertia_tensor;  

    void * _data;
    srph_sdf_func _phi;
} srph_sdf;

void srph_sdf_create(srph_sdf * sdf, srph_sdf_func phi, void * data);
void srph_sdf_full_create(
    srph_sdf * sdf, srph_sdf_func phi, void * data, double volume, 
    srph::mat3_t * inertia_tensor
);

double srph_sdf_phi(srph_sdf * sdf, const vec3 * x);
vec3 srph_sdf_normal(srph_sdf * sdf, const vec3 * x);
srph::mat3_t srph_sdf_jacobian(srph_sdf * sdf, const vec3 * x);
double srph_sdf_volume(srph_sdf * sdf);
double srph_sdf_project(srph_sdf * sdf, const vec3 * d);
bool srph_sdf_contains(srph_sdf * sdf, const vec3 * x);
srph_bound3 * srph_sdf_bound(srph_sdf * sdf);
vec3 srph_sdf_com(srph_sdf * sdf);
srph::mat3_t srph_sdf_inertia_tensor(srph_sdf * sdf);

#endif
