#include "metaphysics/substance.h"

#include <iostream>

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
    srph_matter_to_global_position(&self->matter, &com, srph_matter_com(&self->matter));
    vec3_subtract(r, x, &com);
}

void substance_velocity_at(substance_t *self, const vec3 * x, vec3 * v) {
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
    srph_matter_inverse_inertia_tensor(&self->matter, &i);
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
    vec3_multiply_f(&dv, &n, j_length * srph_matter_inverse_mass(&self->matter));
    vec3_add(&self->matter.v, &self->matter.v, &dv);

    vec3 r, rn, dw;
    offset_from_centre_of_mass(self, &r, x);
    vec3_cross(&rn, &r, &n);

    mat3 i;
    vec3 irn;
    srph_matter_inverse_inertia_tensor(&self->matter, &i);
    vec3_multiply_mat3(&irn, &rn, &i);
    vec3_multiply_f(&dw, &irn, j_length);
    vec3_add(&self->matter.omega, &self->matter.omega, &dw);
}

void substance_apply_impulse(substance_t *a, substance_t *b, const vec3 * x, const vec3 * j) {
    vec3 j_negative;
    vec3_negative(&j_negative, j);

    apply_impulse(a, x, j);
    apply_impulse(b, x, &j_negative);
}
