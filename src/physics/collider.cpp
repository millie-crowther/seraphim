#include "physics/collider.h"

collider_t::collider_t(const sdf_t& sdf){
    this->sdf = sdf;
} 

collider_t::intersection_t
collider_t::collide(const collider_t& c) const {
    sdf_t intersection = sdf.intersection(c.sdf);

    intersection_t result;

    return result;
} 