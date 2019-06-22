#ifndef OCTREE_H
#define OCTREE_H

#include <vector>
#include <memory>
#include <map>

#include "core/buffer.h"
#include "sdf/sdf.h"
#include "render/brick.h"

/*

LXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
01234567 01234567 01234567 01234567

L = is leaf node
if L:
    X = pointer to brick data
else: 
    X = pointer to first child
    if X = 0: 
        there is no child; signal CPU to stream in data.

+ a null node can be described by the literal 0

*/

class octree_t {
private:
    static constexpr uint32_t is_leaf_flag = 1 << 31;
    static constexpr uint32_t null_node = 0;

    static constexpr uint32_t max_structure_size = 25000;
    static constexpr uint32_t max_geometry_size  = 10000;

    std::vector<uint32_t> structure;
    
    /*
        brick data
    */
    // GPU-side
    std::vector<brick_t::data_t> gpu_bricks;

    // CPU-side
    std::vector<brick_t> bricks;

    std::shared_ptr<texture_manager_t> texture_manager;

    std::unique_ptr<buffer_t> buffer;
    std::vector<std::weak_ptr<sdf3_t>> universal_sdfs;
    vec4_t universal_aabb;

    // TODO: remove this and replace with lazy streaming version
    void paint(
        uint32_t i, const vec4_t & aabb, 
        const std::vector<std::shared_ptr<sdf3_t>> & sdfs
    );

    uint32_t create_brick(const vec3_t & x, const sdf3_t & sdf);

public:
    // TODO: sort this out
    octree_t(
        VmaAllocator allocator, VkCommandPool pool, VkQueue queue, double render_distance, 
        const std::vector<std::weak_ptr<sdf3_t>> & sdfs, 
        const std::vector<VkDescriptorSet> & desc_sets
    );
};


#endif
