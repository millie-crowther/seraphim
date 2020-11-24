#include "physics/transform.h"

using namespace srph;

void transform_t::set_position(const vec3_t & x){
    position = x;
    matrix.reset();
}

void transform_t::translate(const vec3_t & x){
    position += x;
    matrix.reset();
}

void transform_t::rotate(const quat_t & q){
    rotation *= q;
    matrix.reset();
}

vec3_t transform_t::to_local_space(const vec3_t & x) const {
    return rotation.inverse() * (x - position);
}

vec3_t transform_t::to_global_space(const vec3_t & x) const {
    return rotation * x + position;
}

vec3_t transform_t::right() const {
    return rotation * vec::right<double>();
}

vec3_t transform_t::up() const {
    return rotation * vec::up<double>();
}

vec3_t transform_t::forward() const {
    return rotation * vec::forward<double>();
}

void transform_t::recalculate_matrix() {
    f32mat3_t r = mat::cast<float>(rotation.to_matrix());

    f32vec4_t a(r.get_column(0), 0.0f);
    f32vec4_t b(r.get_column(1), 0.0f);
    f32vec4_t c(r.get_column(2), 0.0f);
    f32vec4_t d(mat::cast<float>(position), 1.0f);
    
    matrix = std::make_shared<f32mat4_t>(a, b, c, d);
}

f32mat4_t * transform_t::get_matrix(){
    if (matrix == nullptr){
        recalculate_matrix();
    }

    return matrix.get();
}

vec3_t transform_t::get_position() const {
    return position;
}

quat_t transform_t::get_rotation() const {
    return rotation;
}
