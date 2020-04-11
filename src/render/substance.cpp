#include "render/substance.h"

substance_t::substance_t(std::shared_ptr<sdf3_t> sdf){
    this->sdf = sdf;
}

substance_t::data_t::data_t(){
    root = -1;
}

std::weak_ptr<sdf3_t>
substance_t::get_sdf() const {
    return sdf;
}

substance_t::data_t
substance_t::get_data() const {
    return data;
}

std::weak_ptr<aabb3_t> 
substance_t::get_aabb(){
    if (aabb == nullptr){
        aabb = std::make_shared<aabb3_t>();
        vec3_t s(hyper::kappa);
        create_aabb(aabb3_t(-s, s));
    }

    return aabb;
}

void 
substance_t::create_aabb(const aabb3_t & space){
    auto c = space.centre();
    auto s = space.get_size();
    auto phi = sdf->phi(c);

    if (phi <= 0){
        aabb->capture_sphere(c, phi);
    }

    if (s.chebyshev_norm() >= hyper::epsilon && phi * phi < (s / 2).square_norm()){
        for (uint8_t octant = 0; octant < 8; octant++){
            create_aabb(space.subdivide(octant));
        }
    }
}