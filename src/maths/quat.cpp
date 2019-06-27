#include "maths/quat.h"

quat_t::quat_t() : quat_t(1, 0, 0, 0){}

quat_t::quat_t(double w, double x, double y, double z) : q(w, x, y, z) {
    if (q.square_norm() > constant::epsilon){
        q /= q.norm(); 
    } else {
        q = vec4_t(1, 0, 0, 0);
    }
}

quat_t
quat_t::angle_axis(double angle, const vec3_t & axis){
    vec3_t axis_n = axis.normalise();
    double s = std::sin(angle / 2);
    return quat_t(std::cos(angle / 2), axis_n[0] * s, axis_n[1] * s, axis_n[2] * s);
}

quat_t
quat_t::inverse() const {
    return quat_t(q[0], -q[1], -q[2], -q[3]);
}

quat_t
quat_t::operator*(const quat_t& q) const {
    // TODO
    return quat_t(
        // qs[0] * q.qs[1] + qs[1] * q.qs[0] + qs[2] * q.qs[3] - qs[3] * q.qs[2],
        // qs[0] * q.qs[2] - qs[1] * q.qs[3] + qs[2] * q.qs[0] + qs[3] * q.qs[1],
        // qs[0] * q.qs[3] + qs[1] * q.qs[2] - qs[2] * q.qs[1] + qs[3] * q.qs[0],
        // qs[0] * q.qs[0] - qs[1] * q.qs[1] - qs[2] * q.qs[2] - qs[3] * q.qs[3]
    );
}