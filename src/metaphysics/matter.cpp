#include "metaphysics/matter.h"

#include <assert.h>
#include <math.h>

#include "core/random.h"

#define LINEAR_VELOCITY_REST_THRESHOLD 0.075
#define ANGULAR_VELOCITY_REST_THRESHOLD 0.2

void matter_create(matter_t *m, sdf_t *sdf, material_t *mat, const vec3 *x,
                   bool is_uniform, bool is_static) {
    m->sdf = sdf;
    m->material = mat;
    m->is_uniform = is_uniform;
    m->is_static = is_static;
    m->is_rigid = true;
    m->is_at_rest = false;
    m->has_collided = false;
    m->transform.position = x == NULL ? vec3_zero : *x;
    m->transform.rotation = quat_identity;
    m->velocity = vec3_zero;
    m->angular_velocity = vec3_zero;
    m->force = vec3_zero;
    m->torque = vec3_zero;

    if (m->transform.position.y > -90) {
        m->angular_velocity = {{0.1, 0.1, 0.1}};
    }

    array_create(&m->deformations);
}

void matter_destroy(matter_t *m) {
    while (!array_is_empty(&m->deformations)) {
        free(*m->deformations.last);
        array_pop_back(&m->deformations);
    }
    array_clear(&m->deformations);
}

bool matter_is_at_rest(matter_t *m) {
    double v = vec3_length(&m->velocity);
    double w = vec3_length(&m->angular_velocity);

    return m->has_collided && v <= LINEAR_VELOCITY_REST_THRESHOLD &&
           w <= ANGULAR_VELOCITY_REST_THRESHOLD;
}

double matter_average_density(matter_t *self) {
    if (self->is_uniform) {
        return self->material->density;
    }

    assert(false);

    return 0.0;
}

deform_t *matter_add_deformation(matter_t *self, const vec3 *x, deform_type_t type) {
    // transform position into local space
    vec3 x0;
    matter_to_local_position(self, &x0, x);

    // check if new deformation is too close to any other deformations
    for (size_t i = 0; i < self->deformations.size; i++) {
        deform_t *deform = self->deformations.data[i];
        if (vec3_distance(&x0, &deform->x0) < SERAPHIM_DEFORM_MAX_SAMPLE_DENSITY) {
            return NULL;
        }
    }

    // create new deformation
    deform_t *deform = (deform_t *)malloc(sizeof(deform_t));
    *deform = {
        .x0 = x0,
        .x = *x,
        .v = vec3_zero,
        .p = *x,
        .m = 1.0, // TODO
        .type = type,
    };

    // find average velocity
    for (size_t i = 0; i < self->deformations.size; i++) {
        vec3_add(&deform->v, &deform->v, &self->deformations.data[i]->v);
    }

    if (!array_is_empty(&self->deformations)) {
        vec3_multiply_f(&deform->v, &deform->v,
                        1.0 / (double)self->deformations.size);
    }
    // add to list of deformations
    array_push_back(&self->deformations);
    *self->deformations.last = deform;

    return deform;
}

void matter_to_local_position(matter_t *m, vec3 *tx, const vec3 *x) {
    transform_to_local_position(&m->transform, tx, x);
}

void matter_transformation_matrix(matter_t *m, float *xs) {
    mat4 dxs;
    transform_matrix(&m->transform, &dxs);

    for (int i = 0; i < MAT4_SIZE; i++) {
        xs[i] = (float)dxs.v[i];
    }
}

void matter_to_global_position(const matter_t *m, vec3 *tx, const vec3 *x) {
    transform_to_global_position(&m->transform, tx, x);
}

void matter_to_global_direction(const matter_t *m, const vec3 *position, vec3 *td,
                                const vec3 *d) {
    transform_to_global_direction(&m->transform, td, d);
}

void matter_integrate_forces(matter_t *self, double t, const vec3 *gravity,
                             double mass) {
    assert(!self->is_static && !self->is_at_rest);

    // integrate force
    vec3 d;
    vec3_multiply_f(&d, &self->force, t / mass);
    vec3_add(&self->velocity, &self->velocity, &d);

    // integrate torque
    vec3_multiply_f(&d, &self->torque, t / mass);
    vec3_add(&self->angular_velocity, &self->angular_velocity, &d);

    // reset forces
    vec3_multiply_f(&self->force, gravity, mass);
    self->torque = vec3_zero;
}

void matter_material(matter_t *self, material_t *mat, const vec3 *x) {
    // TODO: sample at point
    *mat = *self->material;
}
