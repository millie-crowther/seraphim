#include "render/octree.h"

#include "core/constant.h"
#include "render/painter.h"
#include "sdf/compose.h"

octree_node_t::octree_node_t(){
    header = node_unused_flag;
}

std::vector<octree_node_t>
octree_node_t::create(const f32vec3_t & x, uint8_t depth, const std::vector<std::shared_ptr<sdf3_t>> & sdfs){
    std::vector<octree_node_t> children;
    vec4_t aabb(x[0], x[1], x[2], 2 * hyper::rho / (1 << depth));
    
    for (uint8_t octant = 0; octant < 8; octant++){
        vec4_t octant_aabb = aabb;
        octant_aabb[3] /= 2;

        vec3_t vertex(aabb[0], aabb[1], aabb[2]);

        for (uint8_t axis = 0; axis < 3; axis++){
            if (octant & (1 << axis)){
                octant_aabb[axis] += octant_aabb[3];
                vertex[axis] += aabb[3];
            }
        }

        children.emplace_back(octant_aabb, vertex, sdfs);
    }

    return children;
}

octree_node_t::octree_node_t(const vec4_t & aabb, const vec3_t & vertex, const std::vector<std::shared_ptr<sdf3_t>> & sdfs){
    header = 0;
    
    std::vector<std::shared_ptr<sdf3_t>> new_sdfs;

    for (auto sdf : sdfs){
        auto intersection = intersects_contains(aabb, sdf);
        // contains
        if (std::get<1>(intersection)){
            header = node_empty_flag;
            // return;
        }

        // intersects
        if (std::get<0>(intersection)){
            new_sdfs.push_back(sdf);
        }
    }

    if (new_sdfs.empty()){
        header = node_empty_flag;
        // return;
    }

    compose::union_t<3> sdf(new_sdfs);

    vec3_t c = vec3_t(aabb[0], aabb[1], aabb[2]) + vec3_t(aabb[3] / 2);

    double p = sdf.phi(c);
    p /= constant::sqrt3 * aabb[3];
    p += 0.5;
    p *= 255;
    p = std::max(0.0, std::min(p, 255.0)); 

    vec3_t n = sdf.normal(c);
    u8vec4_t colour = painter_t<3>().colour(vertex);

    n = (n / 2 + 0.5) * 255;
    u8vec4_t normal(n[0], n[1], n[2], p);
    
    geometry = *reinterpret_cast<uint32_t *>(&normal);
    // this->colour = *reinterpret_cast<uint32_t *>(&colour);
}

std::tuple<bool, bool> 
octree_node_t::intersects_contains(const vec4_t & aabb, std::shared_ptr<sdf3_t> sdf) const {
    double lower_radius = 0.5 * aabb[3];
    double upper_radius = constant::sqrt3 * lower_radius;
    vec3_t c = vec3_t(aabb[0], aabb[1], aabb[2]) + vec3_t(lower_radius);

    double p = sdf->phi(c);

    // containment check upper bound
    if (p <= -upper_radius){
        return std::make_tuple(false, true);
    }

    // intersection check lower bound
    if (std::abs(p) <= lower_radius){
        return std::make_tuple(true, false);
    }

    // intersection check upper bound
    if (p >= upper_radius){
        return std::make_tuple(false, false);
    }

    // containment check precise
    double d = (sdf->normal(c) * p).chebyshev_norm();
    if (p < 0 && d > lower_radius){
        return std::make_tuple(false, true);
    }

    // intersection check precise
    if (d <= lower_radius){
        return std::make_tuple(true, false);
    }

    return std::make_tuple(false, false);
}