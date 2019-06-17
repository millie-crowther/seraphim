#ifndef OCTREE_H
#define OCTREE_H

#include <vector>
#include <memory>

#include "core/buffer.h"
#include "core/aabb.h"
#include "sdf/sdf.h"

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
    static constexpr uint32_t max_geometry_size  = 25000;

    std::vector<uint32_t> structure;
    std::vector<f32vec4_t> geometry;
    std::unique_ptr<buffer_t> buffer;
    std::vector<std::weak_ptr<sdf3_t>> universal_sdfs;
    aabb_t universal_aabb;

    // TODO: move these
    bool contains(std::shared_ptr<sdf3_t> sdf, const aabb_t & aabb) const;
    bool intersects(std::shared_ptr<sdf3_t> sdf, const aabb_t & aabb) const;

    // TODO: remove this and replace with lazy streaming version
    void paint(
        uint32_t i, const aabb_t & aabb, 
        const std::vector<std::shared_ptr<sdf3_t>> & sdfs
    );

    uint32_t get_plane_index(const vec4_t & p);

public:
    // TODO: sort this out
    octree_t(
        VmaAllocator allocator, VkCommandPool pool, VkQueue queue, double render_distance, 
        const std::vector<std::weak_ptr<sdf3_t>> & sdfs, 
        const std::vector<VkDescriptorSet> & desc_sets
    );
};


#endif
