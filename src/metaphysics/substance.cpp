#include "metaphysics/substance.h"

#include <core/random.h>
#include <iostream>

substance_t::substance_t(form_t *form, matter_t *matter, uint32_t id) {
    this->form = *form;
    this->matter = *matter;
    this->id = id;
    this->is_inertia_tensor_valid = false;
    this->is_com_valid = false;
}

data_t substance_t::get_data(const vec3 *eye_position) {
    vec3 r;
    srph_bound3_radius(srph_sdf_bound(matter.sdf), &r);

    vec3 eye;
    matter_to_local_position(&matter, &eye, eye_position);

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

    float f32r[3] = {(float)r.x, (float)r.y, (float)r.z};

    data_t data(near, far, f32r, id);

    matter_transformation_matrix(&matter, data.transform);
    data.sdf_id = matter.sdf->id;
    data.material_id = matter.material.id;

    return data;
}

substance_t::substance_t() {}

bool data_t::comparator_t::operator()(const data_t &a, const data_t &b) const {
    return a.far < b.far && a.id != static_cast<uint32_t>(~0);
}
data_t::data_t() { id = ~0; }

data_t::data_t(float near, float far, const float *r, uint32_t id) {
    this->near = near;
    this->far = far;
    this->id = id;

    for (int i = 0; i < 3; i++) {
        this->r[i] = r[i];
    }
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
    vec3 n;
    vec3_normalize(&n, j);
    double j_length = vec3_length(j);

    if (self->matter.is_static || j_length == 0) {
        return;
    }

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

            bound3_t *b = srph_sdf_bound(self->matter.sdf);
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
                    matter_material(&self->matter, &mat, NULL);
                }

                if (srph_sdf_contains(self->matter.sdf, &x)) {
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

        bound3_t *b = srph_sdf_bound(self->matter.sdf);
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
                matter_material(&self->matter, &mat, NULL);
            }
            vec3_multiply_f(&x, &x, mat.density);

            if (srph_sdf_contains(self->matter.sdf, &x)) {
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
    srph_bound3_midpoint(&self->matter.sdf->bound, &midpoint);
    srph_bound3_radius(&self->matter.sdf->bound, &radius);
    matter_to_global_position(&self->matter, &self->bounding_sphere.c, &midpoint);
    self->bounding_sphere.r =
        vec3_length(&radius) + vec3_length(&self->matter.velocity) * dt;
}
