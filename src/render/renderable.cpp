#include "render/renderable.h"

renderable_t::renderable_t(const sdf_t & sdf, transform_t & master) : sdf(sdf){
    visible = true;

    // this line causes a double free on exit. something wierd going on with ref counting 
    // transform = master.create_servant();
}

bool 
renderable_t::is_visible() const { 
    return visible; 
}

bool
renderable_t::intersects(const aabb_t & aabb) const {
    return sdf(aabb.get_centre()) < aabb.get_upper_radius();
}

bool
renderable_t::contains(const aabb_t & aabb) const {
    return sdf(aabb.get_centre()) < -aabb.get_upper_radius();
}
