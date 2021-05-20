#include "transform.h"

#include <assert.h>
#include <math.h>

void transform_to_local_position(transform_t *tf, vec3 *tx, const vec3 *x) {
    vec3_subtract(tx, x, &tf->position);

    quat qi;
    quat_inverse(&qi, &tf->rotation);
    vec3_multiply_quat(tx, tx, &qi);
}

void transform_to_global_position(const transform_t *tf, vec3 *tx, const vec3 *x) {
    vec3_multiply_quat(tx, x, &tf->rotation);
    assert(isfinite(tx->x) && isfinite(tx->y) && isfinite(tx->z));
    vec3_add(tx, tx, &tf->position);
}

void transform_to_global_direction(const transform_t *tf, vec3 *tx, const vec3 *x) {
    vec3_multiply_quat(tx, x, &tf->rotation);
}

void transform_forward(const transform_t *tf, vec3 *x) {
    transform_to_global_direction(tf, x, &vec3_forward);
}

void transform_up(const transform_t *tf, vec3 *x) {
    transform_to_global_direction(tf, x, &vec3_up);
}

void transform_right(const transform_t *tf, vec3 *x) {
    transform_to_global_direction(tf, x, &vec3_right);
}

void transform_translate(transform_t *tf, const vec3 *x) {
    vec3_add(&tf->position, &tf->position, x);
}

void transform_rotate(transform_t *tf, const quat *q) {
    quat_multiply(&tf->rotation, q, &tf->rotation);
}

void transform_matrix(transform_t *tf, mat4 *xs) {
    mat4_rotation_quat(xs, &tf->rotation);
    mat4_translation(xs, xs, &tf->position);
}

