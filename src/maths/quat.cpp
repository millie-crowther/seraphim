#include "maths/quat.h"

quat_t::quat_t() : quat_t(1.0, 0.0, 0.0, 0.0){}

quat_t::quat_t(double w, double x, double y, double z) : qs(w, x, y, z) {
    if (qs.square_norm() > hyper::epsilon){
        qs /= qs.norm(); 
    } else {
        qs = vec4_t(1, 0, 0, 0);
    }
}

quat_t
quat_t::angle_axis(double angle, const vec3_t & a){
    double s = std::sin(angle / 2);
    return quat_t(std::cos(angle / 2), a[0] * s, a[1] * s, a[2] * s);
}

quat_t
quat_t::inverse() const {
    return quat_t(qs[0], -qs[1], -qs[2], -qs[3]);
}

quat_t
quat_t::operator*(const quat_t & q) const {
    return quat_t(
        qs[0] * q.qs[2] - qs[1] * q.qs[3] + qs[2] * q.qs[0] + qs[3] * q.qs[1],
        qs[0] * q.qs[3] + qs[1] * q.qs[2] - qs[2] * q.qs[1] + qs[3] * q.qs[0],
        qs[0] * q.qs[0] - qs[1] * q.qs[1] - qs[2] * q.qs[2] - qs[3] * q.qs[3],
        qs[0] * q.qs[1] + qs[1] * q.qs[0] + qs[2] * q.qs[3] - qs[3] * q.qs[2]
    );
}


quat_t 
quat_t::operator*=(const quat_t & q){
    qs = (q * *this).qs;
}

vec3_t 
quat_t::operator*(const vec3_t & x) const {
    double wx2 = 2 * qs[0] * qs[1];
    double wy2 = 2 * qs[0] * qs[2];
    double wz2 = 2 * qs[0] * qs[3];
    
    double xx2 = 2 * qs[1] * qs[1];
    double xy2 = 2 * qs[1] * qs[2];
    double xz2 = 2 * qs[1] * qs[3];

    double yy2 = 2 * qs[2] * qs[2];
    double yz2 = 2 * qs[2] * qs[3];

    double zz2 = 2 * qs[3] * qs[3];

    return vec3_t(
        (1 - yy2 - zz2) * x[0] + (xy2 - wz2) * x[1] + (xz2 + wy2) * x[2],
        (xy2 + wz2) * x[0] + (1 - xx2 - zz2) * x[1] + (yz2 + wx2) * x[2],
        (xz2 - wy2) * x[0] + (yz2 - wx2) * x[1] + (1 - xx2 - yy2) * x[2]
    );
}