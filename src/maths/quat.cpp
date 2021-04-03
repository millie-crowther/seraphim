#include "maths/quat.h"

#include <assert.h>
#include <math.h>

#include "maths/vector.h"
#include "core/constant.h"

using namespace srph;

//
//quat_t quat_t::euler_angles(const vec3_t & e){
//    vec3 e1 = { e[0], e[1], e[2] };
//    srph_vec3_normalise(&e1, &e1);
//    return angle_axis(vec::length(e), vec3_t(e1.x, e1.y, e1.z));
//}

void srph_quat_inverse(srph_quat * qi, const srph_quat * q){
    srph_quat_init(qi, -q->x, -q->y, -q->z, q->w);
}

void srph_quat_to_matrix(const srph_quat * q, srph_mat3 * m){
    assert(q != NULL && m != NULL);

    double wx = q->w * q->x;
    double wy = q->w * q->y;
    double wz = q->w * q->z;
    
    double xx = q->x * q->x;
    double xy = q->x * q->y;
    double xz = q->x * q->z;

    double yy = q->y * q->y;
    double yz = q->y * q->z;

    double zz = q->z * q->z;

    m->xs[0] = 1.0 - 2.0 * (yy + zz);
    m->xs[1] = 2.0 * (xy + wz);
    m->xs[2] = 2.0 * (xz - wy);

    m->xs[3] = 2.0 * (xy - wz);
    m->xs[4] = 1.0 - 2.0 * (xx + zz);
    m->xs[5] = 2.0 * (yz + wx);

    m->xs[6] = 2.0 * (xz + wy);
    m->xs[7] = 2.0 * (yz - wx);
    m->xs[8] = 1.0 - 2.0 * (xx + yy);
}

void srph_quat_rotate(const srph_quat * q, vec3 * qx, const vec3 * x){
    assert(q != NULL && x != NULL && qx != NULL);
    
    srph_mat3 m;
    srph_quat_to_matrix(q, &m);
    
    *qx = {{{
        x->x * m.xs[0] + x->y * m.xs[1] + x->z * m.xs[2],
        x->x * m.xs[3] + x->y * m.xs[4] + x->z * m.xs[5],
        x->x * m.xs[6] + x->y * m.xs[7] + x->z * m.xs[8]
    }}};
}

void srph_quat_init(srph_quat * q, double x, double y, double z, double w){
    assert(q != NULL);
    
    *q = {{{x, y, z, w}} };
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
    srph_quat_init(q, _a.raw[0] * s, _a.raw[1] * s, _a.raw[2] * s, cos(angle / 2));
}

void srph_quat_rotate_to(srph_quat * q, const vec3 * from, const vec3 * to){
    assert(q != NULL && from != NULL && to != NULL);

    vec3 _from, _to;
    srph_vec3_normalise(&_from, from);
    srph_vec3_normalise(&_to, to);

    vec3 axis;
    srph_vec3_cross(&axis, &_from, &_to);
    
    double dot = srph_vec3_dot(&_from, &_to);
    double angle;
    if (dot >= 1.0){
        angle = 0;
    } else if (dot <= -1.0){
        angle = srph::constant::pi;
    } else {
        angle = acos(srph_vec3_dot(&_from, &_to));
    }

    srph_quat_angle_axis(q, angle, &axis);
}

void srph_quat_mult(srph_quat *q1q2, const srph_quat *q1, const srph_quat *q2) {
    assert(q1q2 != NULL && q1 != NULL && q2 != NULL);

    srph_quat_init(q1q2,
        q1->w * q2->x + q1->x * q2->w + q1->y * q2->z - q1->z * q2->y,
        q1->w * q2->y - q1->x * q2->z + q1->y * q2->w + q1->z * q2->x,
        q1->w * q2->z + q1->x * q2->y - q1->y * q2->x + q1->z * q2->w,
        q1->w * q2->w - q1->x * q2->x - q1->y * q2->y - q1->z * q2->z
    );
}

void srph_quat_euler_angles(srph_quat *q, const vec3 *e) {
    assert(q != NULL && e != NULL);

    vec3 en;
    srph_vec3_normalise(&en, e);
    srph_quat_angle_axis(q, srph_vec3_length(e), &en);
}
