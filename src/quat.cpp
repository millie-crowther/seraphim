#include "quat.h"

#include <iostream>

quat_t::quat_t() : vec4_t({ 1, 0, 0, 0 }){ }

quat_t::quat_t(const std::array<float, 4>& xs) : quat_t(vec4_t(xs)){ }

quat_t::quat_t(const vec4_t& v){
    vec4_t v_n = v.normalise();
    xs = { v_n[0], v_n[1], v_n[2], v_n[3] };
}

quat_t
quat_t::angle_axis(float angle, const vec3_t& axis){
    vec3_t axis_n = axis.normalise();
    float s = std::sin(angle / 2.0f);
    return quat_t({ std::cos(angle / 2.0f), axis_n[0] * s, axis_n[1] * s, axis_n[2] * s });
}

quat_t
quat_t::look_at(const vec3_t& forward, const vec3_t& up){
/*
    // align forward
    vec3_t f = forward.normalise();
    vec3_t axis = f % vec3_t({ 0, 0, 1 });
    float theta = f.angle(vec3_t({ 0, 0, 1 }));
    if ((f % axis).dot(vec3_t({ 0, 0, 1 })) < 0){
	theta = -theta;
    }
    quat_t q1  angle_axis(theta, axis);

    // align upwards
    vec3_t u = up.normalise();
    vec3_t u_r = q1.conjugate(vec3_t({ 0, 1, 0 }));
    axis = u % u_r;
    theta = u.angle(u_r);
    if ((u % axis).dot(u_r) < 0){
	theta = -theta;
    }
    quat_t q2 = angle_axis(theta, axis);

    return q2.hamilton(q1);
*/
    vec3_t f = -forward.normalise();
    vec3_t s = (f % up).normalise();

    mat3_t m = mat3_t({ s, s % f, -f }).transpose();
    
    return from_matrix(m);
}

quat_t
quat_t::conjugate() const {
    return quat_t({ xs[0], -xs[1], -xs[2], -xs[3] });
}

vec3_t
quat_t::inverse_transform(const vec3_t& v) const {
    return conjugate().transform(v);
}

vec3_t
quat_t::transform(const vec3_t& v) const {
    quat_t q = (*this) * quat_t({ 0, v[0], v[1], v[2] }) * conjugate();
    return q.imaginary_part();
}

quat_t
quat_t::operator*(const quat_t& q) const {
    return hamilton(q);
}

vec3_t
quat_t::operator*(const vec3_t& v) const {
    return transform(v);
}

quat_t
quat_t::identity(){
    return quat_t();
}

quat_t
quat_t::inverse() const {
    return conjugate() / square_length();
}

vec3_t
quat_t::imaginary_part(){
    return vec3_t({ xs[1], xs[2], xs[3] });
}

float
quat_t::real_part(){
    return xs[0];
}

quat_t
quat_t::hamilton(const quat_t& q) const {
    return quat_t({
        xs[0] * q.xs[1] + xs[1] * q.xs[0] + xs[2] * q.xs[3] - xs[3] * q.xs[2],
        xs[0] * q.xs[2] - xs[1] * q.xs[3] + xs[2] * q.xs[0] + xs[3] * q.xs[1],
        xs[0] * q.xs[3] + xs[1] * q.xs[2] - xs[2] * q.xs[1] + xs[3] * q.xs[0],
        xs[0] * q.xs[0] - xs[1] * q.xs[1] - xs[2] * q.xs[2] - xs[3] * q.xs[3]
    });
}

mat3_t
quat_t::to_matrix(){
    mat3_t r({
        -xs[2] * xs[2] - xs[3] * xs[3], 
         xs[1] * xs[2] - xs[0] * xs[3], 
         xs[1] * xs[3] + xs[0] * xs[2],
 
         xs[1] * xs[2] + xs[0] * xs[3], 
        -xs[1] * xs[1] - xs[3] * xs[3], 
         xs[2] * xs[3] - xs[0] * xs[1],

         xs[1] * xs[3] - xs[0] * xs[2], 
         xs[2] * xs[3] + xs[0] * xs[1], 
        -xs[1] * xs[1] - xs[2] * xs[2],      
    });    

    return mat3_t::identity() + 2.0f * r;
}

quat_t
quat_t::from_matrix(const mat3_t& m){
    float w = std::sqrt(1.0f + m[0][0] + m[1][1] + m[2][2]) * 2.0f;
    return quat_t({
        w / 4.0f,
        (m[1][2] - m[2][1]) / w,
        (m[2][0] - m[0][2]) / w,
        (m[0][1] - m[1][0]) / w
    });
}
