#ifndef CALL_AND_RESPONSE_H
#define CALL_AND_RESONSE_H

#include "maths/aabb.h"
#include "maths/vec.h"
#include "substance/substance.h"

class call_t {
private:
    f32vec3_t position;
    float radius;

    uint32_t index;
    uint32_t hash;
    uint32_t substanceID;
    uint32_t status;

public:
    struct comparator_t {
        bool operator()(const call_t & a, const call_t & b) const;
    };

    call_t();

    uint32_t get_substance_ID() const;
    f32vec3_t get_position() const;
    float get_radius() const;
    uint32_t get_index() const;
    uint32_t get_hash() const;

    bool is_valid() const;
};

class response_t {
public:
    response_t();
    response_t(const call_t & call, std::weak_ptr<substance_t> substance);

    const std::array<uint32_t, 8> & get_normals() const;
    const std::array<uint32_t, 8> & get_colours() const;
    u32vec2_t get_node() const;

private:
    u32vec2_t node;
    std::array<uint32_t, 8> normals;
    std::array<uint32_t, 8> colours;

    uint32_t squash(const vec4_t & x) const;    

};

#endif
