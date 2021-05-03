#ifndef CALL_AND_RESPONSE_H
#define CALL_AND_RESPONSE_H

#include "metaphysics/substance.h"

static const uint32_t geometry_pool_size = 1000000;
static const uint32_t texture_pool_size = 1000000;

struct call_t {
    srph::f32vec3_t position;
    float radius;

    uint32_t texture_hash;
    uint32_t geometry_hash;
    uint32_t substanceID;
    uint32_t status;

    uint32_t sdf_id;
    uint32_t material_id;
    uint64_t __unused;

    struct comparator_t {
        bool operator()(const call_t &a, const call_t &b) const;
    };

    call_t();

    bool is_valid() const;
};

uint32_t call_geometry_index(const call_t *call);
uint32_t call_texture_index(const call_t *call);

struct patch_t {
    uint32_t contents;
    uint32_t hash;
    float phi;
    uint32_t normal;
};

struct response_t {
    response_t();
    response_t(const call_t &call, substance_t *substance);

    patch_t patch;
    std::array<uint32_t, 8> normals;
    std::array<uint32_t, 8> colours;

    uint32_t squash(const srph::vec4_t &x) const;
};

#endif
