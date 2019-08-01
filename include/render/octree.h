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
        f32vec4_t aabb;
        
        uint32_t child;
        uint32_t parent;
        uint32_t state;
        uint32_t _2;

        request_t(){
            state = request_state_unused;
        }
    };

    struct node_t {
        uint8_t type;
        uint8_t _[3];

        uint32_t b;
        uint32_t c;
        uint32_t d;
    };

    // constants
    static constexpr uint8_t node_type_unused = 1 << 3;
    static constexpr uint8_t node_type_empty  = 1 << 6;
    static constexpr uint8_t node_type_leaf   = 1 << 5;
    static constexpr uint8_t node_type_branch = 1 << 4;

    static constexpr uint8_t request_state_unused = 1;
    static constexpr uint8_t request_state_pending = 2;
    static constexpr uint8_t request_state_fulfilled = 3;
    
    static constexpr uint32_t max_structure_size = 25000;
    static constexpr uint32_t max_requests_size  = 64;

    // fields
    std::unique_ptr<buffer_t> octree_buffer;
    std::unique_ptr<buffer_t> request_buffer;
    std::vector<std::weak_ptr<sdf3_t>> sdfs;
    f32vec4_t universal_aabb;

    // private functions
    std::tuple<bool, bool> intersects_contains(const f32vec4_t & aabb, std::shared_ptr<sdf3_t> sdf) const;
    node_t create_node(const f32vec4_t & aabb);
    void handle_request(const request_t & x);

    VkCommandPool pool;
    VkQueue queue;
    VkDevice device;

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
