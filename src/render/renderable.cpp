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
    return (sdf & aabb.get_sdf())(aabb.get_centre()) < aabb.get_upper_radius();
    // double centre = sdf(aabb.get_centre());
    
    // if (centre < aabb.get_size() / 2){
    //     return true;
    // }

    // if (centre >= aabb.get_upper_radius()){
    //     return false;
    // }

    // // TODO: implement a version that doesnt miss corners
    // return false;
}

bool
renderable_t::contains(const aabb_t & aabb) const {
    double centre = sdf(aabb.get_centre());
    
    if (centre > -aabb.get_size() / 2){
        return false;
    }

    if (centre <= -aabb.get_upper_radius()){
        return true;
    }

    // TODO: implement a version that doesnt miss corners and doesnt assume a convex sdf
    // for (int i = 0; i < 8; i++){
    //     vec3_t x = aabb.get_min();

    //     if (i & 1) x[0] += aabb.get_size();
    //     if (i & 2) x[1] += aabb.get_size();
    //     if (i & 4) x[2] += aabb.get_size();

    //     if (!contains(x)){
    //         return false; 
    //     }
    // }

    return true;
}

bool 
renderable_t::contains(const vec3_t & x) const {
    return sdf(x) <= constant::epsilon;
}