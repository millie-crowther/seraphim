#ifndef OCTREE_H
#define OCTREE_H

#include <memory>
#include <map>
#include <vector>

#include "core/buffer.h"
#include "render/camera.h"
#include "render/texture_manager.h"
#include "sdf/sdf.h"

/*
interior node = 0PPPPPPP PPPPPPPP PPPPPPPP PPPPPPPP
    0 = leaf flag (not set)
    P = pointer to first child
    if P = 0:
        this is a null node

leaf node     = 1NXXXXXX BBBBBBBB BBBBBBBB BBBBBBBB
    1 = leaf flag (set)
    N = normal flag 
    X = unused
    B = brick ID

*/

class octree_t {
private:
    // types
    struct request_t {
        f32vec3_t x;
        uint32_t i;
    };

    struct node_t {
        uint32_t a;
        uint32_t b;
        uint32_t c;
        uint32_t d;
    };

    // constants
    static constexpr uint32_t is_leaf_flag       = 1 << 31;
    static constexpr uint32_t normal_flag        = 1 << 30;
    static constexpr uint32_t brick_id_mask      = 0xFFFFFF;
    static constexpr uint32_t max_structure_size = 25000;
    static constexpr uint32_t max_requests_size  = 64;

    // fields
    std::vector<node_t> structure; // TODO: should this be an array instead?
    std::shared_ptr<texture_manager_t> texture_manager;
    std::unique_ptr<buffer_t> octree_buffer;
    std::unique_ptr<buffer_t> request_buffer;
    std::vector<std::weak_ptr<sdf3_t>> sdfs;
    vec4_t universal_aabb;

    // private functions
    uint32_t lookup(const f32vec3_t & x, uint32_t i, vec4_t & aabb) const;
    std::tuple<bool, bool> intersects_contains(const vec4_t & aabb, std::shared_ptr<sdf3_t> sdf) const;
    node_t create_node(const vec4_t & aabb, uint32_t index);
    void handle_request(const f32vec3_t & x);

public:
    octree_t(
        const allocator_t & allocator, 
        const std::vector<std::weak_ptr<sdf3_t>> & sdfs, 
        const std::vector<VkDescriptorSet> & desc_sets
    );

    void handle_requests();
};


#endif
