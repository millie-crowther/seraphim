#ifndef SERAPHIM_MATTER_H
#define SERAPHIM_MATTER_H

#include "material.h"

#include "maths/sdf/sdf.h"
#include "physics/constraint.h"
#include "physics/sphere.h"
#include "physics/transform.h"

typedef struct srph_matter {
    // fields for rigidbodies
    srph_transform transform;
    vec3 v;
    vec3 omega;

    srph_material material;
    srph_sdf * sdf;

    srph_deform * origin;
    srph_deform * com;
    srph_array(srph_deform *) deformations;

    bool is_uniform;
    bool is_at_rest;
    bool is_rigid;
    bool is_static;

    bool is_inverse_inertia_tensor_valid;
    mat3 inverse_inertia_tensor;

    vec3 f;        
    vec3 t;
} srph_matter;

void srph_matter_init(
    srph_matter * m, srph_sdf * sdf, const srph_material * mat, const vec3 * x, 
    bool is_uniform, bool is_static
);
void srph_matter_destroy(srph_matter * m);

void srph_matter_linear_velocity(const srph_matter *self, vec3 * v);

double srph_matter_inverse_angular_mass(srph_matter * self, vec3 * x, vec3 * n);
double srph_matter_mass(srph_matter * self);

void srph_matter_sphere_bound(const srph_matter * self, double dt, srph_sphere * s);

//void srph_matter_update_vertices(srph_matter * m, double t);
//void srph_matter_extrapolate_next_position_and_velocity(srph_matter * m, double t);
//void srph_matter_resolve_internal_constraints(const srph_matter * m);

void srph_matter_to_global_position(const srph_matter * m, vec3 * tx, const vec3 * x);
void srph_matter_to_local_position(srph_matter *m, vec3 * tx, const vec3 * x);
void srph_matter_to_global_direction(const srph_matter * m, const vec3 * position, vec3 * td, const vec3 * d);

srph_deform * srph_matter_add_deformation(srph_matter * self, const vec3 * x, srph_deform_type type);

void srph_matter_transformation_matrix(srph_matter * m, float * xs);

bool srph_matter_is_inert(srph_matter * m);

void srph_matter_integrate_forces(srph_matter *self, double t, const vec3 *gravity);

void srph_matter_velocity(srph_matter *self, const vec3 *x, vec3 *v);
void srph_matter_material(srph_matter *self, srph_material *mat);

void srph_matter_apply_impulse(srph_matter *self, const vec3 *x, const vec3 *n, double j);

//void srph_matter_rotate(srph_matter *self, quat *q);
bool srph_matter_is_at_rest(srph_matter *self);
void inverse_inertia_tensor(srph_matter *self, mat3 * ri);

double srph_matter_inverse_mass(srph_matter *self);

#endif
