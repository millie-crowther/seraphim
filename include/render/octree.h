#ifndef OCTREE_H
#define OCTREE_H

#include <vector>
#include <memory>

#include "renderable.h"

/*

01234567 01234567 01234567 01234567

B
01234567 01234567 01234567 01234567

B = is described by a brick

*/

class octree_t {
private:
    std::vector<uint64_t> structure;
    std::vector<std::weak_ptr<renderable_t>> universal_renderables;
    aabb_t universal_aabb;

    // predicates
    bool is_empty(const aabb_t & aabb, const std::vector<std::weak_ptr<renderable_t>> & renderables) const;
    bool is_homogenous(const aabb_t & aabb, const std::vector<std::weak_ptr<renderable_t>> & renderables) const;
    bool is_leaf(const vec3_t & x, const vec3_t & camera, const aabb_t & aabb) const;

    int get_node(const vec3_t & x, aabb_t * aabb) const;
    int get_node_helper(const vec3_t & x, int i, aabb_t * aabb) const;

    void request_helper(
        const vec3_t & x, const vec3_t & camera, 
        const aabb_t & aabb, 
        const std::vector<std::weak_ptr<renderable_t>> & renderables
    );

public:
    void request(const vec3_t & x, const vec3_t & camera);
};


#endif
