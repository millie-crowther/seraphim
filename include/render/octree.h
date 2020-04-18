#ifndef OCTREE_H
#define OCTREE_H

#include <memory>

#include "sdf/sdf.h"

/*
    ------UE -------- -------- CCCCCCCC
    XXXXXXXX YYYYYYYY ZZZZZZZZ PPPPPPPP
*/

class octree_node_t {
private:
    static constexpr uint32_t node_empty_flag  = 1 << 24;
    static constexpr uint32_t node_unused_flag = 1 << 25;

    uint32_t header;
    uint32_t geometry;
    
    bool intersects(const vec3_t & c, const vec3_t & r, std::shared_ptr<sdf3_t> sdf) const;

public:
    octree_node_t();
    octree_node_t(const vec3_t & c, const vec3_t & r, const vec3_t & vertex, std::shared_ptr<sdf3_t> sdf);

    static std::vector<octree_node_t> create(const vec3_t & c, const vec3_t & r, std::weak_ptr<sdf3_t> sdf);
};


#endif