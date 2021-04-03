#include "physics/transform.h"

#include <assert.h>

//void srph_transform::set_position(const vec3_t & x){
//    position = x;
//    matrix.reset();
//}
//
//void srph_transform::translate(const vec3_t & x){
//    position += x;
//    matrix.reset();
//}
//
//void srph_transform::rotate(const quat_t & q){
//    rotation *= q;
//    matrix.reset();
//}
//
//vec3_t srph_transform::to_local_space(const vec3_t & x) const {
//    return rotation.inverse() * (x - position);
//}
//
void srph_transform_to_local_position(srph_transform * tf, vec3 * tx, const vec3 * x){
    assert(tf != NULL && tx != NULL && x != NULL);

    srph_vec3_subtract(tx, x, &tf->position);

    srph_quat qi;
    srph_quat_inverse(&qi, &tf->rotation);
    srph_quat_rotate(&qi, tx, tx);
}

void srph_transform_to_global_position(const srph_transform * tf, vec3 * tx, const vec3 * x){
    assert(tf != NULL && tx != NULL && x != NULL);

    srph_quat_rotate(&tf->rotation, tx, x);
    srph_vec3_add(tx, tx, &tf->position);
}
//
//void srph_transform_to_global_space(srph_transform * tf, vec3 * tx, const vec3 * x){
//    vec3_t x1 = tf->to_global_space(vec3_t(x->x, x->y, x->z));
//    *tx = { x1[0], x1[1], x1[2] };
//}
//
//vec3_t srph_transform::to_local_direction(const vec3_t & x) const {
//    vec3 x1 = { x[0], x[1], x[2] };
//    srph_vec3_normalise(&x1, &x1);
//
//
//    return rotation.inverse() * vec3_t(x1.x, x1.y, x1.z);
//}

void srph_transform_to_global_direction(const srph_transform * tf, vec3 * tx, const vec3 * x){
    srph_quat_rotate(&tf->rotation, tx, x);
}

void srph_transform_forward(const srph_transform *tf, vec3 *x) {
    assert(tf != NULL && x != NULL);
    srph_transform_to_global_direction(tf, x, &srph_vec3_forward);
}

void srph_transform_up(const srph_transform *tf, vec3 *x) {
    assert(tf != NULL && x != NULL);
    srph_transform_to_global_direction(tf, x, &srph_vec3_up);
}

void srph_transform_right(const srph_transform *tf, vec3 *x) {
    assert(tf != NULL && x != NULL);
    srph_transform_to_global_direction(tf, x, &srph_vec3_right);
}

void srph_transform_translate(srph_transform *tf, const vec3 *x) {
    assert(tf != NULL && x != NULL);
    srph_vec3_add(&tf->position, &tf->position, x);
}

void srph_transform_rotate(srph_transform *tf, const srph_quat *q) {
    assert(tf != NULL && q != NULL);
    srph_quat_mult(&tf->rotation, &tf->rotation, q);
}

void srph_transform_matrix(srph_transform *tf, double *xs) {
    assert(tf != NULL && xs != NULL);

    vec3 * o = &tf->position;

    srph_mat3 m;
    srph_quat_to_matrix(&tf->rotation, &m);

    xs[0]  = m.xs[0];
    xs[1]  = m.xs[3];
    xs[2]  = m.xs[6];
    xs[3]  = 0.0;

    xs[4]  = m.xs[1];
    xs[5]  = m.xs[4];
    xs[6]  = m.xs[7];
    xs[7]  = 0.0;

    xs[8]  = m.xs[2];
    xs[9]  = m.xs[5];
    xs[10] = m.xs[8];
    xs[11] = 0.0;

    xs[12] = o->x;
    xs[13] = o->y;
    xs[14] = o->z;
    xs[15] = 1.0;
}

//vec3_t srph_transform::to_global_space(const vec3_t & x) const {
//    return rotation * x + position;
//}
//
//
//void srph_transform::recalculate_matrix() {
//    f32mat3_t r = mat::cast<float>(rotation.to_matrix());
//
//    f32vec4_t a(r.get_column(0), 0.0f);
//    f32vec4_t b(r.get_column(1), 0.0f);
//    f32vec4_t c(r.get_column(2), 0.0f);
//    f32vec4_t d(mat::cast<float>(position), 1.0f);
//
//    matrix = std::make_shared<f32mat4_t>(a, b, c, d);
//}
//
//f32mat4_t srph_transform::get_matrix(){
//    if (matrix == nullptr){
//        recalculate_matrix();
//    }
//
//    return *matrix;
//}
//
//vec3_t srph_transform::get_position() const {
//    return position;
//}
//
//quat_t srph_transform::get_rotation() const {
//    return rotation;
//}
