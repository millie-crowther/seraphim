#ifndef SERAPHIM_QUATERNION_H
#define SERAPHIM_QUATERNION_H

#include "vector.h"
#include "matrix.h"

typedef struct srph_quat {
    union {
        struct {
            double x;
            double y;
            double z;
            double w;
        };
        double raw[4];
    };
} srph_quat;

static const srph_quat srph_quat_identity = {{{0.0, 0.0, 0.0, 1.0}}};

void srph_quat_init(srph_quat * q, double x, double y, double z, double w);
void srph_quat_angle_axis(srph_quat * q, double angle, const vec3 * axis);
void srph_quat_rotate_to(srph_quat * q, const vec3 * from, const vec3 * to);
void srph_quat_euler_angles(srph_quat * q, const vec3 * e);

void srph_quat_to_matrix(const srph_quat * q, srph_mat3 * m);
void srph_quat_inverse(srph_quat * qi, const srph_quat * q);
void srph_quat_rotate(const srph_quat * q, vec3 * qx, const vec3 * x);
void srph_quat_mult(srph_quat * q1q2, const srph_quat * q1, const srph_quat * q2);

#endif
