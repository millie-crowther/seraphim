#ifndef SERAPHIM_SDF_H
#define SERAPHIM_SDF_H

#include "../common/array.h"

#include "../common/maths.h"
#include "../common/bound.h"
#include "../common/sphere.h"

#define SERAPHIM_SDF_VOLUME_SAMPLES 10000

typedef struct ray_t {
    vec3 position;
    vec3 direction;
} ray_t;

typedef struct intersection_t {
    vec3f position;
    float distance;

    vec3f direction;
    uint32_t sdf_id;

    vec3f normal;
    float _2;
} intersection_t;

typedef double (*sdf_func_t)(void *data, const vec3 *x);

typedef struct sdf_t {
    uint32_t id;

    bool is_convex;

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

void sdf_create(uint32_t id, sdf_t *sdf, sdf_func_t phi, void *data);

double sdf_distance(sdf_t *sdf, const vec3 *x);
vec3 sdf_normal(sdf_t *sdf, const vec3 *x);
double sdf_volume(sdf_t *sdf);
double sdf_project(sdf_t *sdf, const vec3 *d);
bool sdf_contains(sdf_t *sdf, const vec3 *x);
bound3_t *sdf_bound(sdf_t *sdf);
double sdf_discontinuity(sdf_t *sdf, const vec3 *x);

void sdf_raycast(sdf_t *self, ray_t * ray, intersection_t * intersection);
#endif
