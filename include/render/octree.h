#ifndef OCTREE_H
#define OCTREE_H

#include <array>
#include <vector>
#include <memory>
#include <set>

#include "core/buffer.h"
#include "sdf/sdf.h"
#include "render/brick.h"

class octree_t {
private:
    static constexpr uint32_t is_leaf_flag = 1 << 31;
    static constexpr uint32_t null_node = 0;

    static constexpr uint32_t max_structure_size = 11000;
    static constexpr uint32_t max_brickset_size  = 5000;

    std::vector<uint32_t> structure; // TODO: should this be an array instead?
    
    /*
        brick data
    */
    // device-side
    std::array<brick_t::data_t, max_brickset_size> device_brickset;

    // host-side
    std::set<brick_t> brickset;

    std::shared_ptr<texture_manager_t> texture_manager;

    std::unique_ptr<buffer_t> buffer;
    std::vector<std::weak_ptr<sdf3_t>> universal_sdfs;
    vec4_t universal_aabb;

    // TODO: remove this and replace with lazy streaming version
    void paint(
        uint32_t i, const vec4_t & aabb, 
        const std::vector<std::shared_ptr<sdf3_t>> & sdfs
    );

    uint32_t create_brick(const vec4_t & aabb, const sdf3_t & sdf);

    std::tuple<bool, bool> intersects_contains(const vec4_t & aabb, std::shared_ptr<sdf3_t> sdf) const;

public:
    octree_t(
        const allocator_t & allocator, 
        const std::vector<std::weak_ptr<sdf3_t>> & sdfs, 
        const std::vector<VkDescriptorSet> & desc_sets
    );
};


#endif
