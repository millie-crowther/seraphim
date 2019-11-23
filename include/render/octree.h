#ifndef OCTREE_H
#define OCTREE_H

#include <array>
#include <memory>
#include <tuple>

#include "maths/vec.h"
#include "sdf/sdf.h"

/*
    ------EU -------- -------- CCCCCCCC
    XXXXXXXX YYYYYYYY ZZZZZZZZ PPPPPPPP
*/

class octree_node_t {
private:
    static constexpr uint32_t node_empty_flag  = 1 << 24;
    static constexpr uint32_t node_unused_flag = 1 << 25;

    uint32_t header;
    uint32_t geometry;
    
    std::tuple<bool, bool> intersects_contains(const vec4_t & aabb, std::shared_ptr<sdf3_t> sdf) const;

public:
    octree_node_t();
    octree_node_t(const vec4_t & aabb, const vec3_t & vertex, const std::vector<std::shared_ptr<sdf3_t>> & sdf);

    static std::vector<octree_node_t> create(const f32vec3_t & x, uint8_t depth, const std::vector<std::shared_ptr<sdf3_t>> & sdfs);
};

#endif