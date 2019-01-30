#include "physics/collider.h"

collider_t::collider_t(const sdf_t& sdf){
    this->sdf = sdf;
} 

collider_t::intersection_t
collider_t::collide(const collider_t& c) const {
    sdf_t intersection = sdf.intersection(c.sdf);

    // gradient descent approach to function minimisation
    vec3_t p(0);
    double d = intersection(p);
    vec3_t grad = -intersection.normal(p) * d;
    while (grad.square_length() > constant::epsilon * constant::epsilon){
        p += grad;
        d = intersection(p);
        grad = -intersection.normal(p) * d;
    }

    intersection_t result;
    result.is_hit   = d <= 0;
    result.position = p;
    result.distance = d;

    return result;
} 