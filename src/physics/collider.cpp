#include "physics/collider.h"

collider_t::collider_t(const sdf_t& sdf){
    this->sdf = sdf;
} 

void
collider_t::collide(const collider_t& c) const {
    // TODO
} 