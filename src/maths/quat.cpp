#include "maths/quat.h"

#include <assert.h>

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

void srph_quat_inverse(srph_quat * qi, const srph_quat * q){
    srph_quat_init(qi, -q->x, -q->y, -q->z, q->w);
}

void srph_quat_to_matrix(const srph_quat * q, double * xs){
    assert(q != NULL && xs != NULL);

    double wx = q->w * q->x;
    double wy = q->w * q->y;
    double wz = q->w * q->z;
    
    double xx = q->x * q->x;
    double xy = q->x * q->y;
    double xz = q->x * q->z;

    double yy = q->y * q->y;
    double yz = q->y * q->z;

    double zz = q->z * q->z;

    xs[0] = 2.0 * (0.5 - yy - zz);
    xs[1] = 2.0 * (xy + wz);
    xs[2] = 2.0 * (xz - wy);

    xs[3] = 2.0 * (xy - wz);
    xs[4] = 2.0 * (0.5 - xx - zz);
    xs[5] = 2.0 * (yz + wx);

    xs[6] = 2.0 * (xz + wy);
    xs[7] = 2.0 * (yz - wx);
    xs[8] = 2.0 * (0.5 - xx - yy);
}

void srph_quat_rotate(const srph_quat * q, vec3 * qx, const vec3 * x){
    assert(q != NULL && x != NULL && qx != NULL);
    
    double xs[9];
    srph_quat_to_matrix(q, xs);
    
    *qx = {
        x->x * xs[0] + x->y * xs[1] + x->z * xs[2],
        x->x * xs[3] + x->y * xs[4] + x->z * xs[5],
        x->x * xs[6] + x->y * xs[7] + x->z * xs[8]
    };
}

void srph_quat_init(srph_quat * q, double x, double y, double z, double w){
    assert(q != NULL);
    
    *q = { x, y, z, w };
    double l = sqrt(x * x + y * y + z * z + w * w);
    if (l != 0.0 && l != 1.0){
        for (int i = 0; i < 4; i++){
            q->raw[i] /= l;
        }
    }
}

void srph_quat_angle_axis(srph_quat * q, double angle, const vec3 * a){
    assert(q != NULL && a != NULL);
    
    double s = sin(angle / 2);
    vec3 _a;
    
    srph_vec3_normalise(&_a, a);
    srph_quat_init(q, cos(angle / 2), _a.raw[0] * s, _a.raw[1] * s, _a.raw[2] * s);
}

void srph_quat_rotate_to(srph_quat * q, const vec3 * from, const vec3 * to){
    assert(q != NULL && from != NULL && to != NULL);

    vec3 _from, _to;
    srph_vec3_normalise(&_from, from);
    srph_vec3_normalise(&_to, to);

    vec3 axis;
    srph_vec3_cross(&axis, &_from, &_to);
    
    double angle = acos(srph_vec3_dot(&_from, &_to));
    srph_quat_angle_axis(q, angle, &axis);
}
