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
    [H|D|X] = pointer to first child
    if [H|D|X] = 0: 
        there is no child; signal CPU to stream in data.

+ a null node can be described by the literal 0

*/

class octree_t {
private:
    static constexpr uint32_t is_leaf_flag = 1 << 31;
    static constexpr uint32_t is_homogenous_flag = 1 << 30;
    static constexpr uint32_t brick_pointer_mask = ~(1 << 31 || 1 << 30);
    static constexpr uint32_t child_pointer_mask = ~(1 << 31);
    static constexpr uint32_t null_node = 0;

    std::vector<uint32_t> structure;
    std::vector<std::weak_ptr<renderable_t>> universal_renderables;
    aabb_t universal_aabb;

    bool is_leaf(
        const vec3_t & x, const vec3_t & camera, const aabb_t & aabb, 
        const std::vector<std::weak_ptr<renderable_t>> & renderables
    ) const;

    uint32_t lookup(const vec3_t & x, uint32_t i, aabb_t & aabb) const;

    void subdivide(
        uint32_t i,
        const vec3_t & x, const vec3_t & camera, 
        aabb_t & aabb, 
        const std::vector<std::weak_ptr<renderable_t>> & renderables
    );

public:
    void request(const vec3_t & x, const vec3_t & camera);
};


#endif
