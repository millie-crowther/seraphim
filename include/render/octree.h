#ifndef OCTREE_H
#define OCTREE_H

#include <memory>
#include <map>
#include <vector>

#include "core/buffer.h"
#include "render/camera.h"
#include "sdf/sdf.h"

/*
    node structure
    
    + for all nodes
        + 16 bytes total
        + byte 0 = node type

    + interior nodes
        + byte 1     = lowest byte of number of last frame at which any of 
                       immediate children were observed
        + bytes 8-15 = pointer to first child

    + leaf nodes
        + bytes 4-7  = geometry of node approximant (subject to change)
        + bytes 8-15 = RGBA colour of node


*/

class octree_t {
private:
    // types

    struct request_t {
        f32vec3_t x;
        uint32_t child_24_depth_8;

        request_t(){
            child_24_depth_8 = 0;
        }
    };

    struct node_t {
        uint32_t type;
        uint32_t b;
        uint32_t c;
        uint32_t d;
    };

    struct octree_node_t {
        std::array<node_t, 8> children;
    };

    // constants
    // NOTE: CPU does not know what node type 1 means, only GPU knows.
    //       (it's an internal signal for GPU to its waiting for data from CPU)
    static constexpr uint8_t node_type_unused = 2;
    static constexpr uint8_t node_type_empty  = 3;
    static constexpr uint8_t node_type_leaf   = 4;
    
    static constexpr uint32_t octree_size = 3125;
    static constexpr uint32_t max_requests_size  = 64;

    // fields
    std::unique_ptr<buffer_t> octree_buffer;
    std::unique_ptr<buffer_t> request_buffer;
    std::vector<std::weak_ptr<sdf3_t>> sdfs;
    vec4_t universal_aabb;

    // private functions
    std::tuple<bool, bool> intersects_contains(const vec4_t & aabb, std::shared_ptr<sdf3_t> sdf) const;
    node_t create_node(const vec4_t & aabb);
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
