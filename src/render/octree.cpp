#include "render/octree.h"

void
octree_t::request(const vec3_t & x, const vec3_t & camera){
    if (!universal_aabb.contains(x)){
        return;
    }

    aabb_t aabb = universal_aabb;
    uint32_t i = lookup(x, 0, aabb);

    // node already has adequate data
    if (structure[i] != null_node){
        return;
    }

    // remove invisible renderables
    std::vector<std::weak_ptr<renderable_t>> renderables;
    for (auto & renderable_ptr : universal_renderables){
        if (auto renderable = renderable_ptr.lock()){
            if (renderable->is_visible() && renderable->intersects(aabb)){
                renderables.push_back(renderable_ptr);
            }
        }
    }
    
    subdivide(i, x, camera, aabb, renderables);
}

uint32_t
octree_t::lookup(const vec3_t & x, uint32_t i, aabb_t & aabb) const {
    // base cases
    if (structure[i] == null_node || (structure[i] & is_leaf_flag)) {
        return i;
    } 
    
    // refine octant
    int octant = aabb.get_octant(x);
    aabb.refine(octant);
   
    // tail recurse
    return lookup(x, structure[i] + octant, aabb);
}

void 
octree_t::subdivide(
    uint32_t i,
    const vec3_t & x, const vec3_t & camera, 
    aabb_t & aabb,
    const std::vector<std::weak_ptr<renderable_t>> & renderables
){
    // create pointer to final element in structure
    structure[i] = structure.size();

    // check for homogenous volume
    bool is_empty = true;
    bool is_homogenous = false;
    for (auto & renderable_ptr : renderables){
        if (auto renderable = renderable_ptr.lock()){
            if (renderable->intersects(aabb)){
                is_empty = false;
            }

            if (renderable->contains(aabb)){
                is_homogenous = true;
                break;
            }
        }
    }

    if (is_empty || is_homogenous){
        uint32_t node = is_leaf_flag | is_homogenous_flag;

        if (is_homogenous){
            node |= 1;
        }

        structure.push_back(node);
        return;
    }

    if (is_leaf(x, camera, aabb, renderables)){
        // TODO
        structure.push_back(is_leaf_flag | is_homogenous_flag);
        return;
    }

    // create children
    structure.resize(structure.size() + 8, null_node);

    int octant = aabb.get_octant(x);
    aabb.refine(octant);

    std::vector<std::weak_ptr<renderable_t>> new_renderables;
    for (auto & renderable_ptr : renderables){
        if (auto renderable = renderable_ptr.lock()){
            if (renderable->intersects(aabb)){
                new_renderables.push_back(renderable_ptr);
            }
        }
    }

    subdivide(structure[i] + octant, x, camera, aabb, new_renderables); 
}

bool 
octree_t::is_leaf(
    const vec3_t & x, const vec3_t & camera, const aabb_t & aabb, 
    const std::vector<std::weak_ptr<renderable_t>> & renderables
) const {
    // TODO
    return aabb.get_size() <= 0.1;
}