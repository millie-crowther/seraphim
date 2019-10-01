#ifndef REQUEST_MANAGER_H
#define REQUEST_MANAGER_H

#include <memory>
#include <map>
#include <vector>

#include "core/buffer.h"
#include "render/octree.h"
#include "sdf/sdf.h"

class request_manager_t {
private:
    // types
    struct request_t {
        f32vec3_t x;
        uint32_t child;
    
        uint32_t depth;
        uint32_t unused2;
        uint32_t renderable;
        uint32_t unused3;

        request_t(){
            child = 0;
        }
    };

    static constexpr uint32_t octree_size = 3125;

    // fields
    std::unique_ptr<buffer_t> octree_buffer;
    std::unique_ptr<buffer_t> request_buffer;
    std::vector<std::weak_ptr<sdf3_t>> sdfs;
    std::vector<request_t> requests;

    VkCommandPool pool;
    VkQueue queue;
    VkDevice device;

public:
    request_manager_t(
        VmaAllocator allocator,
        std::shared_ptr<device_t> device,
        const std::vector<std::weak_ptr<sdf3_t>> & sdfs, 
        const std::vector<VkDescriptorSet> & desc_sets, VkCommandPool pool, VkQueue queue,
        uint32_t requests_size
    );

    void handle_requests();
};


#endif
