#include "maths/quat.h"

#include "core/hyper.h"

quat_t::quat_t() : quat_t(1.0, 0.0, 0.0, 0.0){}

quat_t::quat_t(double w, double x, double y, double z) : qs(w, x, y, z) {
    if (qs.square_norm() > hyper::epsilon){
        qs /= qs.norm(); 
    } else {
        qs = vec4_t(1.0, 0.0, 0.0, 0.0);
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
quat_t::operator*(const quat_t & r) const {
    return quat_t(
        r.qs[0] * qs[0] - r.qs[1] * qs[1] - r.qs[2] * qs[2] - r.qs[3] * qs[3],
        r.qs[0] * qs[1] + r.qs[1] * qs[0] - r.qs[2] * qs[3] + r.qs[3] * qs[2],
        r.qs[0] * qs[2] + r.qs[1] * qs[3] + r.qs[2] * qs[0] - r.qs[3] * qs[1],
        r.qs[0] * qs[3] - r.qs[1] * qs[2] + r.qs[2] * qs[1] + r.qs[3] * qs[0]
    );
}

void 
quat_t::operator*=(const quat_t & r){
    qs = (r * *this).qs;
}

vec3_t 
quat_t::operator*(const vec3_t & x) const {
    double wx = qs[0] * qs[1];
    double wy = qs[0] * qs[2];
    double wz = qs[0] * qs[3];
    
    double xx = qs[1] * qs[1];
    double xy = qs[1] * qs[2];
    double xz = qs[1] * qs[3];

    double yy = qs[2] * qs[2];
    double yz = qs[2] * qs[3];

    double zz = qs[3] * qs[3];

    return vec3_t(
        (0.5 - yy - zz) * x[0] + (xy - wz) * x[1] + (xz + wy) * x[2],
        (xy + wz) * x[0] + (0.5 - xx - zz) * x[1] + (yz + wx) * x[2],
        (xz - wy) * x[0] + (yz - wx) * x[1] + (0.5 - xx - yy) * x[2]
    ) * 2;
}