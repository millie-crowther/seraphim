#include "physics/rigidbody.h"

void 
rigidbody_t::add_child(transform_t & child){
    child.set_parent(transform);
}

void
rigidbody_t::add_force(const vec3_t & f){
    add_force_at(f, vec3_t());
}

void 
rigidbody_t::add_force_at(const vec3_t & f, const vec3_t & s){
    
}