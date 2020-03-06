#include "render/octree.h"

#include "core/constant.h"
#include "render/painter.h"
#include "sdf/compose.h"
#include "sdf/primitive.h"

octree_node_t::octree_node_t(){
    header = node_unused_flag;
}

std::vector<octree_node_t>
octree_node_t::create(const f32vec4_t & aabb, std::weak_ptr<sdf3_t> weak_sdf){
    std::vector<octree_node_t> children;

    // TODO: handle case that weak sdf cannot be acquired
    std::shared_ptr<sdf3_t> sdf = weak_sdf.lock();
    
    for (uint8_t octant = 0; octant < 8; octant++){
        vec4_t octant_aabb = aabb.cast<double>();
        octant_aabb[3] /= 2;

        vec3_t vertex(aabb[0], aabb[1], aabb[2]);

        for (uint8_t axis = 0; axis < 3; axis++){
            if (octant & (1 << axis)){
                octant_aabb[axis] += octant_aabb[3];
                vertex[axis] += aabb[3];
            }
        }

        children.emplace_back(octant_aabb, vertex, sdf);
    }

    return children;
}

octree_node_t::octree_node_t(const vec4_t & aabb, const vec3_t & vertex, std::shared_ptr<sdf3_t> sdf){
    header = 0;

    if (intersects(aabb, sdf)){
        header = node_empty_flag;
    }

    vec3_t c = vec3_t(aabb[0], aabb[1], aabb[2]) + vec3_t(aabb[3] / 2);

    double p = sdf->phi(c);
    p /= constant::sqrt3 * aabb[3];
    p += 0.5;
    p *= 255;
    p = std::max(0.0, std::min(p, 255.0)); 

    vec3_t n = sdf->normal(c);
    u8vec4_t colour = painter_t<3>().colour(vertex);

    n = (n / 2 + 0.5) * 255;
    u8vec4_t normal(n[0], n[1], n[2], p);
    
    geometry = *reinterpret_cast<uint32_t *>(&normal);
    // this->colour = *reinterpret_cast<uint32_t *>(&colour);
}

bool
octree_node_t::intersects(const vec4_t & aabb, std::shared_ptr<sdf3_t> sdf) const {
    double lower_radius = 0.5 * aabb[3];
    double upper_radius = constant::sqrt3 * lower_radius;
    vec3_t c = vec3_t(aabb[0], aabb[1], aabb[2]) + vec3_t(lower_radius);

    double p = sdf->phi(c);

    // containment check upper bound
    if (p <= -upper_radius){
        return true;
    }

    // intersection check lower bound
    if (std::abs(p) <= lower_radius){
        return false;
    }

    // intersection check upper bound
    if (p >= upper_radius){
        return true;
    }

    // containment check precise
    double d = (sdf->normal(c) * p).chebyshev_norm();
    if (p < 0 && d > lower_radius){
        return true;
    }

    // intersection check precise
    if (d <= lower_radius){
        return false;
    }

    return true;
}