#include "physics/transform.h"

transform_t::transform_t(){
    matrix = nullptr;
}

void 
transform_t::set_position(const vec3_t & x){
    position = x;
    matrix.reset();
}

void 
transform_t::translate(const vec3_t & x){
    position += x;
    matrix.reset();
}

void 
transform_t::rotate(const quat_t & q){
    rotation *= q;
    matrix.reset();
}

vec3_t 
transform_t::to_local_space(const vec3_t & x) const {
    return rotation.inverse() * (x - position);
}

vec3_t
transform_t::right() const {
    return rotation * vec::right<double>();
}

vec3_t
transform_t::up() const {
    return rotation * vec::up<double>();
}

vec3_t
transform_t::forward() const {
    return rotation * vec::forward<double>();
}

void
transform_t::recalculate_matrix() {
    quat_t inverse = rotation.inverse();
    f32mat3_t r = mat::cast<float>(inverse.to_matrix());

    r = mat::transpose(r);

    auto a = f32vec4_t(r.get_column(0), 0.0f);
    auto b = f32vec4_t(r.get_column(1), 0.0f);
    auto c = f32vec4_t(r.get_column(2), 0.0f);

    f32vec3_t x = mat::cast<float>(rotation * position);

    f32vec4_t d(-x, 1.0f);
    
    matrix = std::make_unique<f32mat4_t>(a, b, c, d);
}

f32mat4_t *
transform_t::get_matrix(){
    if (matrix == nullptr){
        recalculate_matrix();
    }

    return matrix.get();
}

vec3_t 
transform_t::get_position() const {
    return position;
}

quat_t

transform_t::get_rotation() const {
    return rotation;
}
