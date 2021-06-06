#include "metaphysics.h"

#include "../common/random.h"
#include <assert.h>

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

substance_t::substance_t(form_t *form, matter_t *matter, uint32_t id) {
    this->form = *form;
    this->matter = *matter;
    this->id = id;
    this->is_inertia_tensor_valid = false;
    this->is_com_valid = false;
}

substance_t::substance_t() {}

bool substance_data_t::comparator_t::operator()(const substance_data_t &a, const substance_data_t &b) const {
    return a.far < b.far && a.id != static_cast<uint32_t>(~0);
}

substance_data_t::substance_data_t() {
    id = ~0;
    material_id = ~0;
    sdf_id = ~0;
}

static void offset_from_centre_of_mass(substance_t *self, vec3 *r, const vec3 *x) {
    vec3 com;
    matter_to_global_position(&self->matter, &com, substance_com(self));
    vec3_subtract(r, x, &com);
}

void substance_velocity_at(substance_t *self, const vec3 *x, vec3 *v) {
    vec3 r;
    offset_from_centre_of_mass(self, &r, x);
    vec3_cross(v, &self->matter.angular_velocity, &r);
    vec3_add(v, v, &self->matter.velocity);
}

double substance_inverse_angular_mass(substance_t *self, vec3 *x, vec3 *n) {
    if (self->matter.is_static) {
        return 0;
    }

    vec3 r, rn, irn;
    offset_from_centre_of_mass(self, &r, x);
    vec3_cross(&rn, &r, n);
    mat3 i;
    substance_inverse_inertia_tensor(self, &i);
    vec3_multiply_mat3(&irn, &rn, &i);

    return vec3_dot(&rn, &irn);
}

void apply_impulse(substance_t *self, const vec3 *x, const vec3 *j) {
    double j_length = vec3_length(j);

    if (self->matter.is_static || j_length == 0) {
        return;
    }

    vec3 n;
    vec3_normalize(&n, j);

    self->matter.is_at_rest = false;

    vec3 dv;
    vec3_multiply_f(&dv, &n, j_length * substance_inverse_mass(self));
    vec3_add(&self->matter.velocity, &self->matter.velocity, &dv);

    vec3 r, rn, dw;
    offset_from_centre_of_mass(self, &r, x);
    vec3_cross(&rn, &r, &n);

    mat3 i;
    vec3 irn;
    substance_inverse_inertia_tensor(self, &i);
    vec3_multiply_mat3(&irn, &rn, &i);
    vec3_multiply_f(&dw, &irn, j_length);
    vec3_add(&self->matter.angular_velocity, &self->matter.angular_velocity, &dw);
}

void substance_apply_impulse(substance_t *a, substance_t *b, const vec3 *x,
                             const vec3 *j) {
    vec3 j_negative;
    vec3_negative(&j_negative, j);

    apply_impulse(a, x, j);
    apply_impulse(b, x, &j_negative);
}

double substance_inverse_mass(substance_t *self) {
    if (self->matter.is_static) {
        return 0;
    } else {
        return 1.0 / substance_mass(self);
    }
}

void substance_inverse_inertia_tensor(substance_t *self, mat3 *ri) {
    if (self->matter.is_static) {
        return;
    }

    mat3 *i = substance_inertia_tensor(self);

    mat3 r, rt;
    mat3_rotation_quat(&r, &self->matter.transform.rotation);
    mat3_transpose(&rt, &r);

    mat3_multiply(ri, i, &rt);
    mat3_multiply(ri, &r, ri);
    mat3_inverse(ri, ri);
}

mat3 *substance_inertia_tensor(substance_t *self) {
    if (!self->is_inertia_tensor_valid) {
        if (self->matter.is_uniform && self->matter.sdf->is_inertia_tensor_valid) {
            self->inertia_tensor = self->matter.sdf->inertia_tensor;
        } else {
            for (int i = 0; i < 9; i++) {
                self->inertia_tensor.v[i] = 0.0;
            }

            bound3_t *b = sdf_bound(self->matter.sdf);
            random_t rng;
            srph_random_default_seed(&rng);
            int hits = 0;
            double total = 0.0;
            material_t mat;
            matter_material(&self->matter, &mat, NULL);

            while (hits < SERAPHIM_SDF_VOLUME_SAMPLES) {
                vec3 x;
                x.x = srph_random_f64_range(&rng, b->lower.x, b->upper.x);
                x.y = srph_random_f64_range(&rng, b->lower.y, b->upper.y);
                x.z = srph_random_f64_range(&rng, b->lower.z, b->upper.z);

                if (!self->matter.is_uniform) {
                    matter_material(&self->matter, &mat, &x);
                }

                if (sdf_contains(self->matter.sdf, &x)) {
                    for (int i = 0; i < 3; i++) {
                        for (int j = 0; j < 3; j++) {
                            vec3 r;
                            vec3_subtract(&r, &x, substance_com(self));

                            double iij = -r.v[i] * r.v[j];

                            if (i == j) {
                                iij += vec3_length_squared(&r);
                            }

                            self->inertia_tensor.v[j * 3 + i] += iij * mat.density;
                        }
                    }

                    hits++;
                    total += mat.density;
                }
            }

            mat3_multiply_f(&self->inertia_tensor, &self->inertia_tensor,
                            1.0 / total);

            if (self->matter.is_uniform &&
                !self->matter.sdf->is_inertia_tensor_valid) {
                self->matter.sdf->inertia_tensor = self->inertia_tensor;
                self->matter.sdf->is_inertia_tensor_valid = true;
            }
        }

        mat3_multiply_f(&self->inertia_tensor, &self->inertia_tensor,
                        substance_mass(self));
        self->is_inertia_tensor_valid = true;
    }

    return &self->inertia_tensor;
}

vec3 *substance_com(substance_t *self) {
    if (self->matter.is_uniform && self->matter.sdf->is_com_valid) {
        return &self->matter.sdf->com;
    }

    if (!self->is_com_valid) {
        vec3 com = vec3_zero;

        bound3_t *b = sdf_bound(self->matter.sdf);
        random_t rng;
        srph_random_default_seed(&rng);
        int hits = 0;
        double total = 0.0;
        material_t mat;
        matter_material(&self->matter, &mat, NULL);

        while (hits < SERAPHIM_SDF_VOLUME_SAMPLES) {
            vec3 x;
            x.x = srph_random_f64_range(&rng, b->lower.x, b->upper.x);
            x.y = srph_random_f64_range(&rng, b->lower.y, b->upper.y);
            x.z = srph_random_f64_range(&rng, b->lower.z, b->upper.z);

            if (!self->matter.is_uniform) {
                matter_material(&self->matter, &mat, &x);
            }

            if (sdf_contains(self->matter.sdf, &x)) {
                vec3_multiply_f(&x, &x, mat.density);
                vec3_add(&com, &com, &x);
                hits++;
                total += mat.density;
            }
        }

        vec3_divide_f(&com, &com, total);
        self->com = com;
        self->is_com_valid = true;

        if (self->matter.is_uniform && !self->matter.sdf->is_com_valid) {
            self->matter.sdf->com = com;
            self->matter.sdf->is_com_valid = true;
        }
    }

    return &self->com;
}

double substance_mass(substance_t *self) {
    return matter_average_density(&self->matter) * sdf_volume(self->matter.sdf);
}

void substance_calculate_sphere_bound(substance_t *self, double dt) {
    vec3 midpoint, radius;
    bound3_midpoint(&self->matter.sdf->bound, &midpoint);
    bound3_radius(&self->matter.sdf->bound, &radius);
    matter_to_global_position(&self->matter, &self->bounding_sphere.c, &midpoint);
    self->bounding_sphere.r =
        vec3_length(&radius) + vec3_length(&self->matter.velocity) * dt;
}



void substance_data(substance_t *substance, substance_data_t *data, vec3 *eye_position) {
    matter_t * matter = &substance->matter;
    vec3 r;
    bound3_radius(sdf_bound(matter->sdf), &r);

    vec3 eye;
    matter_to_local_position(matter, &eye, eye_position);

    vec3_abs(&eye, &eye);

    vec3 x;
    vec3_subtract(&x, &eye, &r);

    for (int i = 0; i < 3; i++) {
        x.v[i] = fmax(x.v[i], 0.0);
    }

    float near = (float)vec3_length(&x);

    x = eye;
    vec3_add(&x, &eye, &r);

    float far = (float)vec3_length(&x);

    vec3f f32r = {{(float)r.x, (float)r.y, (float)r.z}};

    data->near = near;
    data->far = far;
    data->id = substance->id;
    data->r = f32r;
    data->sdf_id = matter->sdf->id;
    data->material_id = matter->material->id;

    matter_transformation_matrix(matter, data->transform);
}
