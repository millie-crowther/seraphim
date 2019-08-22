#include "render/octree.h"

#include "core/constant.h"
#include "render/painter.h"
#include "sdf/compose.h"

octree_node_t::octree_data_t::octree_data_t(){
    type = node_type_unused;
}

octree_node_t::octree_node_t(){
    header = node_unused_flag;
}

octree_node_t::octree_node_t(const f32vec3_t & x, uint8_t depth, const std::vector<std::shared_ptr<sdf3_t>> & sdfs){
    this->x = x;
    header = depth;
    
    // TODO: this is so hacky!!!! fix this!!!!
    // problem is that the width should be 2 * rho / (1 << depth)
    if (depth > 0){
        depth --;
    }
    
    for (uint8_t octant = 0; octant < 8; octant++){
        vec4_t aabb(x[0], x[1], x[2], hyper::rho / (1 << (depth)));

        if (octant & 1) aabb[0] += aabb[3];
        if (octant & 2) aabb[1] += aabb[3];
        if (octant & 4) aabb[2] += aabb[3];

        children[octant] = octree_data_t(aabb, sdfs);
    }
}

octree_node_t::octree_data_t::octree_data_t(const vec4_t & aabb, const std::vector<std::shared_ptr<sdf3_t>> & sdfs){
    child = 0;
    
    std::vector<std::shared_ptr<sdf3_t>> new_sdfs;

    for (auto sdf : sdfs){
        auto intersection = intersects_contains(aabb, sdf);
        // contains
        if (std::get<1>(intersection)){
            type = node_type_empty;
            return;
        }

        // intersects
        if (std::get<0>(intersection)){
            new_sdfs.push_back(sdf);
        }
    }

    if (new_sdfs.empty()){
        type = node_type_empty;
        return;
    }

    compose::union_t<3> sdf(new_sdfs);
    
    vec3_t c = vec3_t(aabb[0], aabb[1], aabb[2]) + vec3_t(aabb[3] / 2);

    double p = sdf.phi(c);
    p /= constant::sqrt3 * aabb[3];
    p += 0.5;
    p *= 255;
    p = std::max(0.0, std::min(p, 255.0)); 

    vec3_t n = sdf.normal(c);
    u8vec4_t colour = painter_t<3>().colour(c - n * sdf.phi(c));

    n = (n / 2 + 0.5) * 255;
    u8vec4_t normal(n[0], n[1], n[2], p);
    
    type = 0;
    geometry = *reinterpret_cast<uint32_t *>(&normal);
    this->colour = *reinterpret_cast<uint32_t *>(&colour);
}

std::tuple<bool, bool> 
octree_node_t::octree_data_t::intersects_contains(const vec4_t & aabb, std::shared_ptr<sdf3_t> sdf) const {
double upper_radius = vec3_t(aabb[3] / 2).norm();
    vec3_t c = vec3_t(aabb[0], aabb[1], aabb[2]) + vec3_t(aabb[3] / 2.0f);
    double p = sdf->phi(c);

    // containment check upper bound
    if (p <= -upper_radius){
        return std::make_tuple(false, true);
    }

    // intersection check lower bound
    if (std::abs(p) <= aabb[3] / 2){
        return std::make_tuple(true, false);
    }

    // intersection check upper bound
    if (p >= upper_radius){
        return std::make_tuple(false, false);
    }

    // containment check precise
    double d = (sdf->normal(c) * p).chebyshev_norm();
    if (p < 0 && d > aabb[3] / 2){
        return std::make_tuple(false, true);
    }

    // intersection check precise
    if (d <= aabb[3] / 2){
        return std::make_tuple(true, false);
    }

    return std::make_tuple(false, false);
}

