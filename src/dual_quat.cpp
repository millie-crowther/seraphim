#include "dual_quat.h"
#include <iostream>

dual_quat_t::dual_quat_t(const quat_t& r, const quat_t& d){
    real = r.square_length() > 0 ? quat_t(r.normalise()) : r;
    dual = d;
    dual[0] = 0;
}

dual_quat_t::dual_quat_t() : dual_quat_t(quat_t::identity(), quat_t(vec4_t(0))){ }

dual_quat_t::dual_quat_t(const quat_t& q, const vec3_t& v) 
    : dual_quat_t(q, quat_t(vec4_t({ 0, v[0], v[1], v[2] }))){ }

dual_quat_t
dual_quat_t::identity(){
    return dual_quat_t();
}

vec3_t
dual_quat_t::get_translation() const {
    quat_t q = 2 * (dual * real.conjugate());
    return q.imaginary_part();
}

quat_t
dual_quat_t::get_rotation() const {
    return real;
}

dual_quat_t
dual_quat_t::operator+(const dual_quat_t& dq) const {
    return dual_quat_t(real + dq.real, dual + dq.dual); 
}

dual_quat_t
dual_quat_t::operator*(const dual_quat_t& dq) const {
    return dual_quat_t(real * dq.real, real * dq.dual + dual * dq.real);
}

dual_quat_t
dual_quat_t::conjugate() const {
    return dual_quat_t(real.conjugate(), dual.conjugate());
}

vec3_t
dual_quat_t::transform(const vec3_t& v) const {
    dual_quat_t dq = dual_quat_t(quat_t::identity(), v);
    dual_quat_t r = (*this) * dq * conjugate();
    return r.get_translation();
}

vec3_t
dual_quat_t::operator*(const vec3_t& v) const {
    return transform(v);
}

mat4_t
dual_quat_t::to_matrix() const {
    mat3_t r = real.to_matrix();
    vec3_t t = get_translation();

    return mat4_t({
        r[0][0], r[1][0], r[2][0], t[0],
	r[0][1], r[1][1], r[2][1], t[1],
	r[0][2], r[1][2], r[2][2], t[2],
	0,       0,       0,       1
    });
}
