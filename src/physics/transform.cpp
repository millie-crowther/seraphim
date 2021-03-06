#include "physics/transform.h"

using namespace srph;

void srph_transform::set_position(const vec3_t & x){
    position = x;
    matrix.reset();
}

void srph_transform::translate(const vec3_t & x){
    position += x;
    matrix.reset();
}

void srph_transform::rotate(const quat_t & q){
    rotation *= q;
    matrix.reset();
}

vec3_t srph_transform::to_local_space(const vec3_t & x) const {
    return rotation.inverse() * (x - position);
}

void srph_transform_to_local_space(srph_transform * tf, vec3 * tx, const vec3 * x){
    vec3 p = { tf->position[0], tf->position[1], tf->position[2] };
    srph_vec3_subtract(tx, x, &p);
    
    vec3_t tx1 = tf->rotation.inverse() * vec3_t(tx->x, tx->y, tx->z);
    *tx = { tx1[0], tx1[1], tx1[2] };
}

void srph_transform_to_global_space(srph_transform * tf, vec3 * tx, const vec3 * x){
    vec3_t x1 = tf->to_global_space(vec3_t(x->x, x->y, x->z));
    *tx = { x1[0], x1[1], x1[2] };
}

vec3_t srph_transform::to_local_direction(const vec3_t & x) const {
    return rotation.inverse() * vec::normalise(x);
}

vec3_t srph_transform::to_global_space(const vec3_t & x) const {
    return rotation * x + position;
}

vec3_t srph_transform::right() const {
    return rotation * vec::right<double>();
}

vec3_t srph_transform::up() const {
    return rotation * vec::up<double>();
}

vec3_t srph_transform::forward() const {
    return rotation * vec::forward<double>();
}

void srph_transform::recalculate_matrix() {
    f32mat3_t r = mat::cast<float>(rotation.to_matrix());

    f32vec4_t a(r.get_column(0), 0.0f);
    f32vec4_t b(r.get_column(1), 0.0f);
    f32vec4_t c(r.get_column(2), 0.0f);
    f32vec4_t d(mat::cast<float>(position), 1.0f);
    
    matrix = std::make_shared<f32mat4_t>(a, b, c, d);
}

f32mat4_t srph_transform::get_matrix(){
    if (matrix == nullptr){
        recalculate_matrix();
    }

    return *matrix;
}

vec3_t srph_transform::get_position() const {
    return position;
}

quat_t srph_transform::get_rotation() const {
    return rotation;
}
