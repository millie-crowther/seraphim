#include "physics/ray.h"

#include "core/constant.h"

ray_t::ray_t(const vec3_t& o, const vec3_t& d){
    origin    = o;
    direction = d;
}

ray_t::intersection_t
ray_t::cast(const sdf_t& sdf) const {
    vec3_t p = origin;
    double d = sdf(p);
    double total = d;

    intersection_t result;
 
    for (int i = 0; d > constant::epsilon && total < constant::rho && i < max_iterations; i++){
        d = sdf(p);
        total += d;
        p += d * direction;
    }

    result.is_hit   = d <= constant::epsilon;
    result.position = p;
    result.normal   = sdf.normal(p);
    
    return result;  
}