#include "physics/transform.h"

#include <assert.h>
#include <math.h>

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
    vec3_subtract(tx, x, &tf->position);

    quat qi;
    quat_inverse(&qi, &tf->rotation);
    vec3_multiply_quat(tx, tx, &qi);
}

void srph_transform_to_global_position(const srph_transform * tf, vec3 * tx, const vec3 * x){
    vec3_multiply_quat(tx, x, &tf->rotation);
    assert(isfinite(tx->x) && isfinite(tx->y) && isfinite(tx->z));
    vec3_add(tx, tx, &tf->position);
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
    vec3_multiply_quat(tx, x, &tf->rotation);
}

void srph_transform_forward(const srph_transform *tf, vec3 *x) {
    srph_transform_to_global_direction(tf, x, &vec3_forward);
}

void srph_transform_up(const srph_transform *tf, vec3 *x) {
    srph_transform_to_global_direction(tf, x, &vec3_up);
}

void srph_transform_right(const srph_transform *tf, vec3 *x) {
    srph_transform_to_global_direction(tf, x, &vec3_right);
}

void srph_transform_translate(srph_transform *tf, const vec3 *x) {
    vec3_add(&tf->position, &tf->position, x);
}

void srph_transform_rotate(srph_transform *tf, const quat *q) {
    quat_multiply(&tf->rotation, q,  &tf->rotation);
}

void srph_transform_matrix(srph_transform *tf, mat4 *xs) {
    mat4_rotation_quat(xs, &tf->rotation);
    mat4_translation(xs, xs, &tf->position);
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
