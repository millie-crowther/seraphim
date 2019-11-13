#ifndef OCTREE_H
#define OCTREE_H

#include <array>
#include <memory>
#include <tuple>

#include "maths/vec.h"
#include "sdf/sdf.h"

class octree_node_t {
private:
    class octree_data_t {
    private:
        static constexpr uint8_t node_empty_flag  = 1 << 0;
        static constexpr uint8_t node_unused_flag = 1 << 1;

        uint32_t header;
        uint32_t geometry;
        uint32_t colour;
        uint32_t child;    
        
        std::tuple<bool, bool> intersects_contains(const vec4_t & aabb, std::shared_ptr<sdf3_t> sdf) const;

    public:
        octree_data_t();
        octree_data_t(const vec4_t & aabb, const vec3_t & vertex, const std::vector<std::shared_ptr<sdf3_t>> & sdf);
    };

    static constexpr uint32_t node_unused_flag = 1 << 8;

    f32vec3_t x;
    uint32_t header;
    std::array<octree_data_t, 8> children;  

public:
    octree_node_t(){}
    octree_node_t(const f32vec3_t & x, uint8_t depth, const std::vector<std::shared_ptr<sdf3_t>> & sdfs);
};

#endif