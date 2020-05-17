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
    std::array<u32vec2_t, 8> nodes;
    std::array<uint32_t, 8> normals;
    std::array<uint32_t, 8> colours;

    uint32_t squash(const vec4_t & x) const;    

public:
    response_t(const call_t & call, std::weak_ptr<substance_t> substance);

    const std::array<uint32_t, 8> & get_normals() const;
    const std::array<uint32_t, 8> & get_colours() const;
    std::array<octree_node_t, 8> get_nodes() const;
};

#endif
