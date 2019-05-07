#ifndef OCTREE_H
#define OCTREE_H

#include <vector>
#include <memory>

#include "renderable.h"

/*

LHXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
01234567 01234567 01234567 01234567

L = is leaf node
if L:
    H = homogenous volume flag
    if H:
        X = data describing material (pointer?)
    else:
        X = pointer to brick data
else: 
    [H|X] = pointer to first child
    if [H|X] = 0: 
        there is no child; flag to CPU to stream in data.

+ a null node can be described by the literal 0

*/

class octree_t {
private:
    static constexpr uint32_t is_leaf_flag = 1 << 31;
    static constexpr uint32_t is_homogenous_flag = 1 << 30;
    static constexpr uint32_t brick_pointer_mask = ~(1 << 31 || 1 << 30);
    static constexpr uint32_t child_pointer_mask = ~(1 << 31);

    std::vector<uint32_t> structure;
    std::vector<std::weak_ptr<renderable_t>> universal_renderables;
    aabb_t universal_aabb;

    // predicates
    bool is_empty(const aabb_t & aabb, const std::vector<std::weak_ptr<renderable_t>> & renderables) const;
    bool is_homogenous(const aabb_t & aabb, const std::vector<std::weak_ptr<renderable_t>> & renderables) const;
    bool is_leaf(const vec3_t & x, const vec3_t & camera, const aabb_t & aabb) const;

    int lookup(const vec3_t & x, aabb_t * aabb) const;
    int lookup_helper(const vec3_t & x, int i, aabb_t * aabb) const;

    void request_helper(
        const vec3_t & x, const vec3_t & camera, 
        const aabb_t & aabb, 
        const std::vector<std::weak_ptr<renderable_t>> & renderables
    );

public:
    void request(const vec3_t & x, const vec3_t & camera);
};


#endif
