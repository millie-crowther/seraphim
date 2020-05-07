#include "render/octree.h"

#include "core/constant.h"
#include "render/painter.h"
#include "sdf/compose.h"
#include "sdf/primitive.h"

octree_node_t::octree_node_t(){
    header = node_unused_flag;
}

std::vector<octree_node_t>
octree_node_t::create(const vec3_t & c, const vec3_t & r, std::weak_ptr<sdf3_t> weak_sdf){
    std::vector<octree_node_t> children;

    if (auto sdf = weak_sdf.lock()){
        for (uint8_t o = 0; o < 8; o++){
            vec3_t d = (vec3_t((o & 1) << 1, o & 2, (o & 4) >> 1) - 1).hadamard(r);
            children.emplace_back(c + d / 2, r / 2, c + d, sdf);
        }
    }

    return children;
}

octree_node_t::octree_node_t(const vec3_t & c, const vec3_t & r, const vec3_t & vertex, std::shared_ptr<sdf3_t> sdf){
    header = 0;

    if (!intersects(c, r, sdf)){
        header |= node_empty_flag;
    }

    double p = sdf->phi(c);
    p /= r.norm() * 2;
    p += 0.5;
    p *= 255;
    p = std::max(0.0, std::min(p, 255.0)); 


    vec3_t n = (sdf->normal(c) / 2 + 0.5) * 255;
    
    u8vec4_t normal = vec4_t(n[0], n[1], n[2], p).cast<uint8_t>();
    
    geometry = *reinterpret_cast<uint32_t *>(&normal);
}

bool
octree_node_t::intersects(const vec3_t & c, const vec3_t & r, std::shared_ptr<sdf3_t> sdf) const {
    double lower_radius = r.chebyshev_norm();
    double upper_radius = r.norm();

    double p = sdf->phi(c);

    // 1. is aabb definitely fully inside SDF?
    if (p <= -upper_radius){
        return false;
    }

    // 2. is it possible that part of aabb is partially outside SDF?
    if (std::abs(p) <= lower_radius){
        return true;
    }

    // 3. is aabb definitely fully outside SDF?
    if (p >= upper_radius){
        return false;
    }

    // 4. same as test 1 but more expensive and precise
    double d = (sdf->normal(c) * p).chebyshev_norm();
    if (p < 0 && d > lower_radius){
        return false;
    }

    // 5. same as test 2 but more precise (again, i think)
    if (d <= lower_radius){
        return true;
    }

    // 6. default case
    return false;
}

std::array<uint32_t, 8>
octree_node_t::get_normals(const vec3_t & c, const vec3_t & r, std::weak_ptr<sdf3_t> weak_sdf){
    std::array<uint32_t, 8> normals;

    if (auto sdf = weak_sdf.lock()){
        for (uint8_t o = 0; o < 8; o++){
            vec3_t d = (vec3_t((o & 1) << 1, o & 2, (o & 4) >> 1) - 1).hadamard(r);
            u8vec3_t n = sdf->normal(c + d).cast<uint8_t>();
            normals[o] = *reinterpret_cast<uint32_t *>(&n);
        }
    }

    return normals;
}