#ifndef CALL_AND_RESPONSE_H
#define CALL_AND_RESONSE_H

#include "maths/aabb.h"
#include "maths/vec.h"
#include "substance/substance.h"

class call_t {
private:
    f32vec3_t c;
    float size;

    uint32_t child;
    uint32_t unused;
    uint32_t substanceID;
    uint32_t status;

public:
    struct comparator_t {
        bool operator()(const call_t & a, const call_t & b) const;
    };

    call_t();
    call_t(const f32vec3_t & c, float size);

    uint32_t get_substance_ID() const;
    f32vec3_t get_centre() const;
    float get_size() const;
    uint32_t get_child() const;

    bool is_valid() const;
};

class response_t {
public:
    static constexpr uint32_t node_empty_flag = 1 << 24;
    static constexpr uint32_t node_unused_flag = 1 << 25;
    static constexpr uint32_t node_child_mask = 0xFFFF;


    response_t();
    response_t(const call_t & call, uint32_t octant, std::weak_ptr<substance_t> substance);

    const std::array<uint32_t, 8> & get_normals() const;
    const std::array<uint32_t, 8> & get_colours() const;
    uint32_t get_node() const;

private:
    uint32_t node;
    std::array<uint32_t, 8> normals;
    std::array<uint32_t, 8> colours;

    uint32_t squash(const vec4_t & x) const;    
    void create_node(const vec3_t & c, const vec3_t & r, std::shared_ptr<substance_t> sub);

};

#endif
