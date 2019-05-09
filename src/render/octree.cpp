#include "render/octree.h"

void
octree_t::request(const vec3_t & x, const vec3_t & camera){
    if (!universal_aabb.contains(x)){
        return;
    }

    aabb_t aabb = universal_aabb;
    int index = lookup(x, 0, aabb);

    if (structure[index] != 0){
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
    
    subdivide(index, x, camera, aabb, renderables);
}

int
octree_t::lookup(const vec3_t & x, int i, aabb_t & aabb) const {
    // base cases
    if (structure[i] == 0 || (structure[i] & is_leaf_flag)) {
        return i;
    } 
    
    // refine octant
    int octant = aabb.get_octant(x);
    aabb.refine(octant);
    
    // find child
    int index = (structure[i] & child_pointer_mask) + octant;
   
    // tail recurse
    return lookup(x, index, aabb);
}

void 
octree_t::subdivide(
    int index,
    const vec3_t & x, const vec3_t & camera, 
    const aabb_t & aabb,
    const std::vector<std::weak_ptr<renderable_t>> & renderables
){
    if (is_empty(aabb, renderables)){
        // TODO
        return;
    }

    if (is_homogenous(aabb, renderables)){
        // TODO
        return;
    }

    if (is_leaf(x, camera, aabb)){
        // TODO
        return;
    }

    // create children and point to them
    structure[index] = static_cast<uint32_t>(structure.size());
    for (int i = 0; i < 8; i++){
        structure.push_back(0);
    }

    int octant = aabb.get_octant(x);
    uint32_t new_index = structure[i] + octant;
    aabb.refine(octant);

    std::vector<std::weak_ptr<renderable_t>> new_renderables;
    for (auto & renderable_ptr : renderables){
        if (auto renderable = renderable_ptr.lock()){
            if (renderable->intersects(new_aabb)){
                new_renderables.push_back(renderable_ptr);
            }
        }
    }

    subdivide(new_index, x, camera, aabb, new_renderables); 
}

bool 
octree_t::is_empty(const aabb_t & aabb, const std::vector<std::weak_ptr<renderable_t>> & renderables) const {
    if (renderables.empty()){
        return true;
    }

    for (auto & renderable_ptr : renderables){
        if (auto renderable = renderable_ptr.lock()){
            if (renderable->intersects(aabb)){
                return false;
            }
        }
    }

    return true;
}

bool 
octree_t::is_homogenous(const aabb_t & aabb, const std::vector<std::weak_ptr<renderable_t>> & renderables) const {
    for (auto & renderable_ptr : renderables){
        if (auto renderable = renderable_ptr.lock()){
            if (renderable->contains(aabb)){
                return true;
            }
        }
    }

    return false;
}

bool 
octree_t::is_leaf(const vec3_t & x, const vec3_t & camera, const aabb_t & aabb) const {
    // TODO
    return aabb.get_size() <= 0.1;
}
