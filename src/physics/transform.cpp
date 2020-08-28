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
    return rotation.inverse() * (position - x);
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

    float wx = inverse[0] * inverse[1];
    float wy = inverse[0] * inverse[2];
    float wz = inverse[0] * inverse[3];
    
    float xx = inverse[1] * inverse[1];
    float xy = inverse[1] * inverse[2];
    float xz = inverse[1] * inverse[3];

    float yy = inverse[2] * inverse[2];
    float yz = inverse[2] * inverse[3];

    float zz = inverse[3] * inverse[3];

    auto a = f32vec4_t(
        0.5f - yy - zz,
        xy - wz,
        xz + wy,
        0.0f
    ) * 2.0f;

    auto b = f32vec4_t(
        xy + wz,
        0.5f - xx - zz,
        yz + wx,
        0.0f
    ) * 2.0f;

    auto c = f32vec4_t(
        xz - wy,
        yz - wx,
        0.5f - xx - yy,
        0.0f
    ) * 2.0f;

    f32vec3_t x = rotation * position;

    f32vec4_t d(
        -x[0],
        -x[1],
        -x[2],
        1.0f
    );

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
