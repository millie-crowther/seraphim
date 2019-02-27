#include "scene/transform.h"

transform_t::transform_t() {
    this->parent = nullptr;
}


transform_t::transform_t(transform_t * parent){
    this->parent = parent;
}

std::shared_ptr<transform_t> 
transform_t::create_child(){
    std::shared_ptr<transform_t> child(this);
    children.push_back(child);
    return child;
}

void
transform_t::set_position(const vec3_t & x){
    this->position = x;
}

void 
transform_t::set_rotation(const quat_t & q){
    this->rotation = q;
}