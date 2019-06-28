#include "scene/transform.h"

// transform_t::transform_t(){
    // master = nullptr;
// }

// std::weak_ptr<transform_t>
// transform_t::create_servant(){
//     std::shared_ptr<transform_t> servant = std::make_shared<transform_t>();
//     servant->master = this;
//     servants.push_back(servant);
//     return servant;
// }

void 
transform_t::set_position(const vec3_t & x){
    position = x;
}

void 
transform_t::translate(const vec3_t & x){
    position += x;
}

void 
transform_t::set_rotation(const quat_t & q){
    rotation = q;
}

void 
transform_t::rotate(const quat_t & q){
    rotation *= q;
}

quat_t 
transform_t::get_rotation() const {
    return rotation;
}

vec3_t
transform_t::get_position() const {
    return position;
}