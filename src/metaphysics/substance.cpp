#include "metaphysics/substance.h"

#include <iostream>
#include <core/random.h>

using namespace srph;

substance_t::substance_t(form_t * form, matter_t * matter, uint32_t id) {
	this->form = *form;
	this->matter = *matter;
	this->id = id;
}

data_t substance_t::get_data(const vec3 * eye_position) {
	vec3 r;
	srph_bound3_radius(srph_sdf_bound(matter.sdf), &r);

	vec3 eye;
	srph_matter_to_local_position(&matter, &eye, eye_position);

	vec3_abs(&eye, &eye);

	vec3 x;
	vec3_subtract(&x, &eye, &r);

	for (int i = 0; i < 3; i++) {
		x.v[i] = fmax(x.v[i], 0.0);
	}

	float near = (float) vec3_length(&x);

	x = eye;
	vec3_add(&x, &eye, &r);

	float far = (float) vec3_length(&x);

	data_t data(near, far, f32vec3_t(r.x, r.y, r.z), id);

	srph_matter_transformation_matrix(&matter, data.transform);
	data.sdf_id = matter.sdf->id;

	return data;
}

substance_t::substance_t() {

}

bool data_t::comparator_t::
operator           () (const data_t & a, const data_t & b)
	 const {
		 return a.far < b.far && a.id != static_cast < uint32_t > (~0);
	 }
data_t::data_t() {
	id = ~0;
}

data_t::data_t(float near, float far, const f32vec3_t & r, uint32_t id) {
	this->near = near;
	this->far = far;
	this->r = r;
	this->id = id;
}

static void offset_from_centre_of_mass(substance_t * self, vec3 * r, const vec3 * x) {
	vec3 com;
	srph_matter_to_global_position(&self->matter, &com,
		srph_matter_com(&self->matter));
	vec3_subtract(r, x, &com);
}

void substance_velocity_at(substance_t * self, const vec3 * x, vec3 * v) {
	vec3 r;
	offset_from_centre_of_mass(self, &r, x);
	vec3_cross(v, &self->matter.omega, &r);
	vec3_add(v, v, &self->matter.v);
}

double substance_inverse_angular_mass(substance_t * self, vec3 * x, vec3 * n) {
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

void apply_impulse(substance_t * self, const vec3 * x, const vec3 * j) {
	vec3 n;
	vec3_normalize(&n, j);
	double j_length = vec3_length(j);

	if (self->matter.is_static || j_length == 0) {
		return;
	}

	self->matter.is_at_rest = false;

	vec3 dv;
	vec3_multiply_f(&dv, &n, j_length * substance_inverse_mass(self));
	vec3_add(&self->matter.v, &self->matter.v, &dv);

	vec3 r, rn, dw;
	offset_from_centre_of_mass(self, &r, x);
	vec3_cross(&rn, &r, &n);

	mat3 i;
	vec3 irn;
    substance_inverse_inertia_tensor(self, &i);
	vec3_multiply_mat3(&irn, &rn, &i);
	vec3_multiply_f(&dw, &irn, j_length);
	vec3_add(&self->matter.omega, &self->matter.omega, &dw);
}

void substance_apply_impulse(substance_t * a, substance_t * b, const vec3 * x,
	const vec3 * j) {
	vec3 j_negative;
	vec3_negative(&j_negative, j);

	apply_impulse(a, x, j);
	apply_impulse(b, x, &j_negative);
}

double substance_inverse_mass(substance_t * self) {
	if (self->matter.is_static) {
		return 0;
	} else {
		return 1.0 / srph_matter_mass(&self->matter);
	}
}


void substance_inverse_inertia_tensor(substance_t *self, mat3 * ri) {
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


mat3 *substance_inertia_tensor(substance_t *blingblong) {
    if (!blingblong->matter.is_inertia_tensor_valid) {
        if (blingblong->matter.is_uniform && blingblong->matter.sdf->is_inertia_tensor_valid) {
            blingblong->matter.inertia_tensor = blingblong->matter.sdf->inertia_tensor;
        } else {
            for (int i = 0; i < 9; i++) {
                blingblong->matter.inertia_tensor.v[i] = 0.0;
            }

            bound3_t *b = srph_sdf_bound(blingblong->matter.sdf);
            random_t rng;
            srph_random_default_seed(&rng);
            int hits = 0;
            double total = 0.0;
            material_t mat;
            srph_matter_material(&blingblong->matter, &mat, NULL);

            while (hits < SERAPHIM_SDF_VOLUME_SAMPLES) {
                vec3 x;
                x.x = srph_random_f64_range(&rng, b->lower.x, b->upper.x);
                x.y = srph_random_f64_range(&rng, b->lower.y, b->upper.y);
                x.z = srph_random_f64_range(&rng, b->lower.z, b->upper.z);

                if (!blingblong->matter.is_uniform) {
                    srph_matter_material(&blingblong->matter, &mat, NULL);
                }

                if (srph_sdf_contains(blingblong->matter.sdf, &x)) {
                    for (int i = 0; i < 3; i++) {
                        for (int j = 0; j < 3; j++) {
                            vec3 r;
                            vec3_subtract(&r, &x, srph_matter_com(&blingblong->matter));

                            double iij = -r.v[i] * r.v[j];

                            if (i == j) {
                                iij += vec3_length_squared(&r);
                            }

                            blingblong->matter.inertia_tensor.v[j * 3 + i]
                                    += iij * mat.density;
                        }
                    }

                    hits++;
                    total += mat.density;
                }
            }

            mat3_multiply_f(&blingblong->matter.inertia_tensor,
                            &blingblong->matter.inertia_tensor, 1.0 / total);

            if (blingblong->matter.is_uniform && !blingblong->matter.sdf->is_inertia_tensor_valid) {
                blingblong->matter.sdf->inertia_tensor = blingblong->matter.inertia_tensor;
                blingblong->matter.sdf->is_inertia_tensor_valid = true;
            }
        }

        mat3_multiply_f(&blingblong->matter.inertia_tensor,
                        &blingblong->matter.inertia_tensor, srph_matter_mass(&blingblong->matter));
        blingblong->matter.is_inertia_tensor_valid = true;
    }

    return &blingblong->matter.inertia_tensor;
}
