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
    double s = maths::inverse_square_root(w * w + x * x + y * y + z * z);
    
    if (s > 0){
        w *= s;
        x *= s;
        y *= s;
        z *= s;
    } else {
        w = 1;
        x = 0;
        y = 0;
        z = 0;
    }
}

quat_t
quat_t::angle_axis(double angle, const vec3_t & axis){
    vec3_t axis_n = axis.normalise();
    double s = std::sin(angle / 2);
    return quat_t(std::cos(angle / 2), axis_n[0] * s, axis_n[1] * s, axis_n[2] * s);
}

vec3_t
quat_t::rotate(const vec3_t & v) const {
    // TODO theres a normalise in here that shouldnt be
    quat_t q = *this * quat_t(0, v[0], v[1], v[2]) * inverse();
    return q.vector();
}

void
quat_t::operator*=(const quat_t & q){
    quat_t q1 = hamilton(q);
    w = q1.w;
    x = q1.x;
    y = q1.y;
    z = q1.z; 
}

quat_t
quat_t::operator*(const quat_t & q) const {
    return hamilton(q);
}

vec3_t
quat_t::operator*(const vec3_t & v) const {
    return rotate(v);
}

quat_t
quat_t::identity(){
    return quat_t();
}

quat_t
quat_t::inverse() const {
    return quat_t(w, -x, -y, -z);
}

vec3_t
quat_t::vector() const {
    return vec3_t(x, y, z);
}

double
quat_t::scalar() const {
    return w;
}

quat_t
quat_t::hamilton(const quat_t& q) const {
    // TODO
    return quat_t(
        // qs[0] * q.qs[1] + qs[1] * q.qs[0] + qs[2] * q.qs[3] - qs[3] * q.qs[2],
        // qs[0] * q.qs[2] - qs[1] * q.qs[3] + qs[2] * q.qs[0] + qs[3] * q.qs[1],
        // qs[0] * q.qs[3] + qs[1] * q.qs[2] - qs[2] * q.qs[1] + qs[3] * q.qs[0],
        // qs[0] * q.qs[0] - qs[1] * q.qs[1] - qs[2] * q.qs[2] - qs[3] * q.qs[3]
    );
}