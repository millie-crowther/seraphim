#ifndef OCTREE_H
#define OCTREE_H

#include <memory>
#include <map>
#include <vector>

#include "core/buffer.h"
#include "render/camera.h"
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
        float size;
        
        u32vec3_t _;
        uint32_t i;
    };

    struct node_t {
        uint8_t type;
        uint8_t _[3];

        uint32_t b;
        uint32_t c;
        uint32_t d;
    };

    // constants
    static constexpr uint8_t node_type_unused = 1 << 7;
    static constexpr uint8_t node_type_empty  = 1 << 6;
    static constexpr uint8_t node_type_leaf   = 1 << 5;
    static constexpr uint8_t node_type_branch = 1 << 4;
    static constexpr uint8_t node_type_stem   = 1 << 3;
    
    static constexpr uint32_t max_structure_size = 25000;
    static constexpr uint32_t max_requests_size  = 64;

    // fields
    std::vector<node_t> structure; // TODO: should this be an array instead?
    std::unique_ptr<buffer_t> octree_buffer;
    std::unique_ptr<buffer_t> request_buffer;
    std::vector<std::weak_ptr<sdf3_t>> sdfs;
    vec4_t universal_aabb;

    // private functions
    uint32_t lookup(const f32vec3_t & x, uint32_t i, vec4_t & aabb) const;
    std::tuple<bool, bool> intersects_contains(const vec4_t & aabb, std::shared_ptr<sdf3_t> sdf) const;
    node_t create_node(const vec4_t & aabb, uint32_t index);
    void handle_request(const f32vec3_t & x);

    VkCommandPool pool;
    VkQueue queue;

public:
    octree_t(
        VmaAllocator allocator,
        std::shared_ptr<device_t> device,
        const std::vector<std::weak_ptr<sdf3_t>> & sdfs, 
        const std::vector<VkDescriptorSet> & desc_sets, VkCommandPool pool, VkQueue queue
    );

    void handle_requests();
};


#endif
