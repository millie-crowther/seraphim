#include "physics/collider.h"

collider_t::collider_t(const sdf_t& sdf){
    this->sdf = sdf;
    colliding = true;
} 

void
collider_t::collide(std::shared_ptr<collider_t> c) const {
    if (colliding && (has_listeners() || c->has_listeners())){
        // TODO
    }
} 

bool
collider_t::intersects_plane(const vec3_t & v, const vec3_t & n) const {
    vec3_t t1 = n.tangent();
    vec3_t t2 = n % t1;

    return true; // TODO
}

bool
collider_t::is_colliding() const {
    return colliding;
}