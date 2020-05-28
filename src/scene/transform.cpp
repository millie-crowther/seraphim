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
