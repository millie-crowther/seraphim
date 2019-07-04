#ifndef OCTREE_H
#define OCTREE_H

#include <memory>
#include <set>
#include <vector>

#include "core/buffer.h"
#include "render/brick.h"
#include "render/camera.h"
#include "sdf/sdf.h"

/*
interior node = 0PPPPPPP PPPPPPPP PPPPPPPP PPPPPPPP
    0 = leaf flag (not set)
    P = pointer to first child
    if P = 0:
        this is a null node

leaf node = 1NDXXXXX BBBBBBBB BBBBBBBB BBBBBBBB
    1 = leaf flag (set)
    N = normal flag 
    D = detail flag
    X = unused
    B = brick ID

*/

class octree_t {
private:
    static constexpr uint32_t is_leaf_flag   = 1 << 31;
    static constexpr uint32_t normal_flag    = 1 << 30;
    static constexpr uint32_t detail_flag    = 1 << 29;
    static constexpr uint32_t brick_ptr_mask = 0xFFFFFF;

    static constexpr uint32_t null_node = 0;

    static constexpr uint32_t max_structure_size = 100000;

    std::vector<uint32_t> structure; // TODO: should this be an array instead?
    
    // TODO: make this a queue for easy LRU elimination 
    std::set<brick_t> brickset; 

    std::shared_ptr<texture_manager_t> texture_manager;

    std::unique_ptr<buffer_t> octree_buffer;
    std::unique_ptr<buffer_t> request_buffer;

    std::vector<std::weak_ptr<sdf3_t>> universal_sdfs;
    vec4_t universal_aabb;

    // TODO: remove this and replace with lazy streaming version
    void paint(
        uint32_t i, const vec4_t & aabb, 
        const std::vector<std::weak_ptr<sdf3_t>> & sdfs,
        std::shared_ptr<camera_t> camera
    );

    std::tuple<bool, bool> intersects_contains(const vec4_t & aabb, std::shared_ptr<sdf3_t> sdf) const;

    bool is_leaf(
        const vec4_t & aabb, 
        const std::vector<std::shared_ptr<sdf3_t>> & sdfs,
        std::shared_ptr<camera_t> camera 
    );

public:
    octree_t(
        const allocator_t & allocator, 
        const std::vector<std::weak_ptr<sdf3_t>> & sdfs, 
        const std::vector<VkDescriptorSet> & desc_sets,
        std::weak_ptr<camera_t> camera
    );
};


#endif
