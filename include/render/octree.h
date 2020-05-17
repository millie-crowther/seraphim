#ifndef OCTREE_H
#define OCTREE_H

#include <memory>

#include "sdf/sdf.h"

/*
    ------UE -------- -------- CCCCCCCC
    XXXXXXXX YYYYYYYY ZZZZZZZZ PPPPPPPP
*/

class octree_node_t {
public:
    static constexpr uint32_t node_empty_flag  = 1 << 24;
    static uint32_t node_unused_flag;
    static constexpr uint32_t node_child_mask = 0xFFFF;

    uint32_t header;
    uint32_t geometry;
    
    bool intersects(const vec3_t & c, const vec3_t & r, std::shared_ptr<sdf3_t> sdf) const;

public:
    octree_node_t();
    octree_node_t(const vec3_t & c, const vec3_t & r, const vec3_t & vertex, std::shared_ptr<sdf3_t> sdf);

};


#endif
