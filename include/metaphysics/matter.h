#ifndef SERAPHIM_MATTER_H
#define SERAPHIM_MATTER_H

#include "material.h"

#include "maths/sdf/sdf.h"
#include "physics/deform.h"
#include "physics/sphere.h"
#include "physics/transform.h"

typedef struct matter_t {
    transform_t transform;
    vec3 velocity;
    vec3 angular_velocity;

    material_t * material;
    sdf_t *sdf;

    array_t(deform_t *) deformations;

    bool is_uniform;
    bool is_at_rest;
    bool is_rigid;
    bool is_static;
    bool has_collided;

    vec3 force;
    vec3 torque;
} matter_t;

void matter_create(matter_t *m, sdf_t *sdf, material_t *mat, const vec3 *x,
                   bool is_uniform, bool is_static);
void matter_destroy(matter_t *m);

void matter_to_global_position(const matter_t *m, vec3 *tx, const vec3 *x);
void matter_to_local_position(matter_t *m, vec3 *tx, const vec3 *x);
void matter_to_global_direction(const matter_t *m, const vec3 *position, vec3 *td,
                                const vec3 *d);

deform_t *matter_add_deformation(matter_t *self, const vec3 *x, deform_type_t type);

void matter_transformation_matrix(matter_t *m, float *xs);

bool matter_is_at_rest(matter_t *m);

void matter_integrate_forces(matter_t *self, double t, const vec3 *gravity,
                             double mass);

void matter_material(matter_t *self, material_t *mat, const vec3 *x);

double matter_average_density(matter_t *self);

#endif
