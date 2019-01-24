#include "physics/ray.h"

#include "maths/constants.h"

ray_t::ray_t(const vec3_t& o, const vec3_t& d){
    origin    = o;
    direction = d;
}

ray_t::intersection_t
ray_t::cast(const sdf_t& sdf) const {
    vec3_t p = origin;
    double d = sdf(p);
    double total = d;

    ray_t::intersection_t result;

    while (d > constants::epsilon && total < constants::rho){
        d = sdf(p);
        total += d;
        p += d * direction;
    }

    result.isHit    = d <= constants::epsilon;
    result.position = p;
    result.normal   = sdf.normal(p);
    
    return result;  
}