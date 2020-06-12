#include "scene/transform.h"

void 
transform_t::set_position(const vec3_t & x){
    const std::lock_guard<std::mutex> lock(position_mutex);
    position = x;
}

void 
transform_t::translate(const vec3_t & x){
    set_position(position + x);
}

void 
transform_t::set_rotation(const quat_t & q){
    const std::lock_guard<std::mutex> lock(rotation_mutex);
    rotation = q;
}

void 
transform_t::rotate(const quat_t & q){
    set_rotation(q * rotation);
}

quat_t 
transform_t::get_rotation() const {
    return rotation;
}

vec3_t
transform_t::get_position() const {
    return position;
}

vec3_t 
transform_t::to_local_space(const vec3_t & x) const {
    return rotation.inverse() * (position - x);
}

f32mat4_t
transform_t::to_matrix() const {
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

    f32vec4_t a = f32vec4_t(
        0.5f - yy - zz,
        xy - wz,
        xz + wy,
        0.0f
    ) * 2.0f;

    f32vec4_t b = f32vec4_t(
        xy + wz,
        0.5f - xx - zz,
        yz + wx,
        0.0f
    ) * 2.0f;

    f32vec4_t c = f32vec4_t(
        xz - wy,
        yz - wx,
        0.5f - xx - yy,
        0.0f
    ) * 2.0f;

    vec3_t x = rotation * position;

    f32vec4_t d = f32vec4_t(
        -x[0],
        -x[1],
        -x[2],
        1.0f
    );

    return f32mat4_t(a, b, c, d);
}