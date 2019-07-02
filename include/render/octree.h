#ifndef OCTREE_H
#define OCTREE_H

#include <array>
#include <vector>
#include <memory>
#include <set>

#include "core/buffer.h"
#include "sdf/sdf.h"
#include "render/brick.h"
#include "render/camera.h"
/*

interior nodes:

node = 0PPPPPPP PPPPPPPP PPPPPPPP PPPPPPPP

    0 = leaf flag (not set)
    P = pointer to first child

leaf nodes:

node = 1NXXXXXX BBBBBBBB BBBBBBBB BBBBBBBB
    1 = leaf flag (set)
    N = normal flag (indicates sign of Z component of normal)
    X = unused (4 bits)
    B = brick ID



*/

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
        const std::vector<std::weak_ptr<sdf3_t>> & sdfs,
        std::shared_ptr<camera_t> camera
    );

    uint32_t create_brick(const vec4_t & aabb, const sdf3_t & sdf);

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
