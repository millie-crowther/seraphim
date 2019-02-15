#include "maths/quat.h"

#include <iostream>

quat_t::quat_t() : quat_t(1, 0, 0, 0){}

quat_t::quat_t(double w, double x, double y, double z){
    this->w = w;
    this->x = x;
    this->y = y;
    this->z = z;
    normalise();
}

void
quat_t::normalise(){
    double s = maths::inverse_square_root(
        w * w + x * x + y * y + z * z
    );
    
    w *= s;
    x *= s;
    y *= s;
    z *= s;
}

quat_t
quat_t::angle_axis(double angle, const vec3_t& axis){
    vec3_t axis_n = axis.normalise();
    float s = std::sin(angle / 2.0f);
    return quat_t(std::cos(angle / 2.0f), axis_n[0] * s, axis_n[1] * s, axis_n[2] * s);
}

vec3_t
quat_t::rotate(const vec3_t& v) const {
    quat_t q = *this * quat_t({ 0, v[0], v[1], v[2] }) * inverse();
    return q.vector();
}

quat_t
quat_t::operator*(const quat_t& q) const {
    return hamilton(q);
}

vec3_t
quat_t::operator*(const vec3_t& v) const {
    return rotate(v);
}

quat_t
quat_t::identity(){
    return quat_t();
}

quat_t
quat_t::inverse() const {
    return quat_t(qs[0], -qs[1], -qs[2], -qs[3]);
}

vec3_t
quat_t::vector() const {
    return vec3_t(qs[1], qs[2], qs[3]);
}

double
quat_t::scalar() const {
    return qs[0];
}

quat_t
quat_t::hamilton(const quat_t& q) const {
    return quat_t(
        qs[0] * q.qs[1] + qs[1] * q.qs[0] + qs[2] * q.qs[3] - qs[3] * q.qs[2],
        qs[0] * q.qs[2] - qs[1] * q.qs[3] + qs[2] * q.qs[0] + qs[3] * q.qs[1],
        qs[0] * q.qs[3] + qs[1] * q.qs[2] - qs[2] * q.qs[1] + qs[3] * q.qs[0],
        qs[0] * q.qs[0] - qs[1] * q.qs[1] - qs[2] * q.qs[2] - qs[3] * q.qs[3]
    );
}