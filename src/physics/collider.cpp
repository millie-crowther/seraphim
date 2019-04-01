#include "physics/collider.h"

collider_t::collider_t(const sdf_t& sdf){
    this->sdf = sdf;
    colliding = true;
} 

void
collider_t::collide(std::shared_ptr<collider_t> c) const {
    if (colliding && c->colliding && (has_followers() || c->has_followers())){
        // TODO
    }
} 

vec3_t
collider_t::get_position() const {
    return transform.get_absolute_position();
}

bool
collider_t::intersects_plane(const vec3_t & v, const vec3_t & n) const {
    sdf_t plane([&v, &n](const vec3_t & x){
        return std::abs((x - v) * n) - constant::epsilon;
    });
    return !(sdf && plane).is_null();
}

bool
collider_t::is_colliding() const {
    return colliding;
}