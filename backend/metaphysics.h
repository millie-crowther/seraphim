#ifndef SERAPHIM_METAPHYSICS_H
#define SERAPHIM_METAPHYSICS_H

#include <memory>
#include "../common/transform.h"
#include "../common/material.h"
#include "sdf.h"
#include "deform.h"
#include "../common/substance_data.h"


typedef struct form_t {

} form_t;

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

typedef struct substance_t {
    uint32_t id;

    bool is_com_valid;
    mat3 inertia_tensor;
    vec3 com;

    bool is_inertia_tensor_valid;
    mat3 inverse_inertia_tensor;

    sphere_t bounding_sphere;

    form_t form;
    matter_t matter;
} substance_t;

void substance_create(substance_t *self, form_t *form, matter_t *matter, uint32_t id);
void substance_data(substance_t *substance, substance_data_t *data, vec3 *eye_position);

// velocity
void substance_velocity_at(substance_t *self, const vec3 *x, vec3 *v);
void substance_apply_impulse(substance_t *a, substance_t *b, const vec3 *x,
                             const vec3 *j);

// angular mass
double substance_inverse_angular_mass(substance_t *self, vec3 *x, vec3 *n);
mat3 *substance_inertia_tensor(substance_t *self);
void substance_inverse_inertia_tensor(substance_t *self, mat3 *ri);

// mass
double substance_mass(substance_t *self);
double substance_inverse_mass(substance_t *self);
vec3 *substance_com(substance_t *self);

void substance_calculate_sphere_bound(substance_t *self, double dt);


#endif
