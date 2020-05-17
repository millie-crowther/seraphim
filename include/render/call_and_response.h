#ifndef CALL_AND_RESPONSE_H
#define CALL_AND_RESONSE_H

#include "maths/aabb.h"
#include "maths/vec.h"
#include "render/octree.h"
#include "render/substance.h"

class call_t {
public:
    f32vec3_t c;
    uint32_t depth;

    uint32_t child;
    uint32_t unused;
    uint32_t substanceID;
    uint32_t status;

public:
    call_t();

    uint32_t get_substance_ID() const;
};

class response_t {
private:
    std::vector<octree_node_t> nodes;
    std::array<uint32_t, 8> normals;
    std::array<uint32_t, 8> colours;

public:
    response_t(const call_t & call, std::weak_ptr<substance_t> substance);

    const std::array<uint32_t, 8> & get_normals() const;
    const std::array<uint32_t, 8> & get_colours() const;
    const std::vector<octree_node_t> & get_nodes() const;
};

#endif
