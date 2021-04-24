#ifndef SERAPHIM_MATTER_H
#define SERAPHIM_MATTER_H

#include "material.h"

#include "maths/sdf/sdf.h"
#include "physics/deform.h"
#include "physics/sphere.h"
#include "physics/transform.h"

typedef struct srph_matter {
    // fields for rigidbodies
    srph_transform transform;
    vec3 v;
    vec3 omega;

    material_t material;
    srph_sdf *sdf;

    srph_array(srph_deform *) deformations;

    bool is_uniform;
    bool is_at_rest;
    bool is_rigid;
    bool is_static;

    bool is_inertia_tensor_valid;
    bool is_com_valid;
    mat3 inertia_tensor;
    vec3 com;

    mat3 inverse_inertia_tensor;

    sphere_t bounding_sphere;

    vec3 f;
    vec3 t;
} srph_matter;

void srph_matter_init(
        srph_matter *m, srph_sdf *sdf, const material_t *mat, const vec3 *x,
        bool is_uniform, bool is_static
);

void srph_matter_destroy(srph_matter *m);

void srph_matter_calculate_sphere_bound(srph_matter *self, double dt);

void srph_matter_to_global_position(const srph_matter *m, vec3 *tx, const vec3 *x);
void srph_matter_to_local_position(srph_matter *m, vec3 *tx, const vec3 *x);

void srph_matter_to_global_direction(const srph_matter *m, const vec3 *position, vec3 *td, const vec3 *d);

srph_deform *srph_matter_add_deformation(srph_matter *self, const vec3 *x, srph_deform_type type);

void srph_matter_transformation_matrix(srph_matter *m, float *xs);

bool srph_matter_is_inert(srph_matter *m);

void srph_matter_integrate_forces(srph_matter *self, double t, const vec3 *gravity);

void srph_matter_velocity(srph_matter *self, const vec3 *x, vec3 *v);

void srph_matter_material(srph_matter *self, material_t *mat, const vec3 *x);

bool srph_matter_is_at_rest(srph_matter *self);

// mass
vec3 *srph_matter_com(srph_matter *matter);
double srph_matter_mass(srph_matter *self);
double srph_matter_inverse_mass(srph_matter *self);

// angular mass
mat3 *srph_matter_inertia_tensor(srph_matter *matter);
void srph_matter_inverse_inertia_tensor(srph_matter *self, mat3 *ri);
double srph_matter_inverse_angular_mass(srph_matter *self, vec3 *x, vec3 *n);
void matter_apply_impulse(srph_matter *a, srph_matter *b, const vec3 *x, const vec3 *j);

#endif
