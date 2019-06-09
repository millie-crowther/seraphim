#include "render/renderable.h"

renderable_t::renderable_t(std::shared_ptr<sdf_t> sdf, transform_t & master) : sdf(sdf){
    visible = true;
    this->sdf = sdf;

    // this line causes a double free on exit. something wierd going on with ref counting 
    // transform = master.create_servant();
}

bool 
renderable_t::is_visible() const { 
    return visible; 
}

bool
renderable_t::intersects(const aabb_t & aabb) const {
    double phi = std::abs(sdf->phi(aabb.get_centre()));
    
    if (phi <= aabb.get_size() / 2){
        return true;
    }

    if (phi >= aabb.get_upper_radius()){
        return false;
    }

    vec3_t x = sdf->normal(aabb.get_centre()) * phi;

    return x.chebyshev_norm() <= aabb.get_size() / 2;
}