#include "physics/rigidbody.h"

rigidbody_t::rigidbody_t(const sdf_t & sdf) : collider(sdf){ }

std::shared_ptr<transform_t> 
rigidbody_t::create_servant(){
    return transform.create_servant();
}

void
rigidbody_t::add_force(const vec3_t & f){
    add_force_at(f, vec3_t());
}

void 
rigidbody_t::add_force_at(const vec3_t & f, const vec3_t & s){
    
}