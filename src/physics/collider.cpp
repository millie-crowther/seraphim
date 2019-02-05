#include "physics/collider.h"

collider_t::collider_t(const sdf_t& sdf){
    this->sdf = sdf;
} 

void
collider_t::collide(std::shared_ptr<collider_t> c) const {
    // TODO
} 

bool 
collider_t::intersects_plane(const vec3_t & v, const vec3_t & n) const {
    return sdf.intersects_plane(v, n); 
}