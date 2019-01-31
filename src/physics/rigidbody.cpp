#include "physics/rigidbody.h"

void 
rigidbody_t::add_child(transform_t& child){
    child.set_parent(transform);
}