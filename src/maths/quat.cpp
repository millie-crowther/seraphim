#include "maths/quat.h"

#include "maths/vector.h"

using namespace srph;

quat_t::quat_t() : quat_t(1.0, 0.0, 0.0, 0.0){}

quat_t::quat_t(double w, double x, double y, double z) : qs(w, x, y, z) {
    double l = sqrt(w * w + x * x + y * y + z * z);
    if (l != 0.0){
        w /= l;
        x /= l;
        y /= l;
        z /= l;
    }
}

quat_t quat_t::angle_axis(double angle, const vec3_t & a){
    double s = std::sin(angle / 2);
    return quat_t(std::cos(angle / 2), a[0] * s, a[1] * s, a[2] * s);
}

quat_t quat_t::euler_angles(const vec3_t & e){
    vec3 e1 = { e[0], e[1], e[2] };
    srph_vec3_normalise(&e1, &e1);
    return angle_axis(vec::length(e), vec3_t(e1.x, e1.y, e1.z));
}

quat_t quat_t::inverse() const {
    return quat_t(qs[0], -qs[1], -qs[2], -qs[3]);
}

quat_t quat_t::operator*(const quat_t & r) const {
    return quat_t(
        r.qs[0] * qs[0] - r.qs[1] * qs[1] - r.qs[2] * qs[2] - r.qs[3] * qs[3],
        r.qs[0] * qs[1] + r.qs[1] * qs[0] - r.qs[2] * qs[3] + r.qs[3] * qs[2],
        r.qs[0] * qs[2] + r.qs[1] * qs[3] + r.qs[2] * qs[0] - r.qs[3] * qs[1],
        r.qs[0] * qs[3] - r.qs[1] * qs[2] + r.qs[2] * qs[1] + r.qs[3] * qs[0]
    );
}

void quat_t::operator*=(const quat_t & r){
    qs = (r * *this).qs;
}

double quat_t::operator[](uint32_t i) const {
    return qs[i];
}

mat3_t quat_t::to_matrix() const {
    double wx = qs[0] * qs[1];
    double wy = qs[0] * qs[2];
    double wz = qs[0] * qs[3];
    
    double xx = qs[1] * qs[1];
    double xy = qs[1] * qs[2];
    double xz = qs[1] * qs[3];

    double yy = qs[2] * qs[2];
    double yz = qs[2] * qs[3];

    double zz = qs[3] * qs[3];
    
    mat3_t r(
        0.5 - yy - zz, xy + wz,       xz - wy,
        xy - wz,       0.5 - xx - zz, yz + wx,
        xz + wy,       yz - wx,       0.5 - xx - yy    
    );

    r *= 2.0;

    return r;
}
