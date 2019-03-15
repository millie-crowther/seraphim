#include "scene/transform.h"

transform_t::transform_t() {
    this->master = nullptr;
}


transform_t::transform_t(transform_t * master){
    this->master = master;
}

std::shared_ptr<transform_t> 
transform_t::create_servant(){
    std::shared_ptr<transform_t> servant(this);
    servants.push_back(servant);
    return servant;
}

void
transform_t::set_position(const vec3_t & x){
    this->position = x;
}

void 
transform_t::set_rotation(const quat_t & q){
    this->rotation = q;
}