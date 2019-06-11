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

bool 
renderable_t::contains(const aabb_t & aabb) const {
    double phi = sdf->phi(aabb.get_centre());
    
    if (phi >= -aabb.get_size() / 2){
        return false;
    }

    if (phi <= -aabb.get_upper_radius()){
        return true;
    }

    vec3_t x = sdf->normal(aabb.get_centre()) * phi;
    return x.chebyshev_norm() >= aabb.get_size() / 2;
}

vec4_t 
renderable_t::plane(const vec3_t & x) const {
    vec3_t n = sdf->normal(x);
    return vec4_t(
        n[0], n[1], n[2],
        (x * n) - sdf->phi(x) // TODO: maths here is fine afaict but comes out weird
    );
}