#include "render/octree.h"

void
octree_t::request(const vec3_t & x, const vec3_t & camera){
    // remove invisible renderables
    std::vector<std::weak_ptr<renderable_t>> visible_renderables;
    for (auto & renderable_ptr : universal_renderables){
        if (auto renderable = renderable_ptr.lock()){
            if (renderable->is_visible()){
                visible_renderables.push_back(renderable_ptr);
            }
        }
    }

    // call recursive helper method at top level
    request_helper(x, camera, universal_aabb, visible_renderables);
}

int
octree_t::lookup(const vec3_t & x, aabb_t * aabb) const {
    if (!universal_aabb.contains(x)){
        return -1;
    } else {
        return lookup_helper(x, 0, universal_aabb, aabb);
    }
}

int
octree_t::lookup_helper(const vec3_t & x, int i, const aabb_t & aabb, aabb_t * output_aabb) const {
    if (structure[i] == 0){
        // subdivide
    } else if (structure[i] & is_leaf_flag) {
        // TODO: set aabb (need to pass it in)
        return i;
    } 
    
    // tail recursion
    int octant; //TODO
    int index = (structure[i] & child_pointer_mask) + aabb.get_octant(octant);
    return lookup_helper(x, index, aabb, output_aabb);
}

void 
octree_t::request_helper(
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

    for (int i = 0; i < 8; i++){
        aabb_t new_aabb = aabb.get_octant(i);

        std::vector<std::weak_ptr<renderable_t>> new_renderables;
        for (auto & renderable_ptr : renderables){
            if (auto renderable = renderable_ptr.lock()){
                if (renderable->intersects(new_aabb)){
                    new_renderables.push_back(renderable_ptr);
                }
            }
        }

        request_helper(x, camera, new_aabb, new_renderables);
    }
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
