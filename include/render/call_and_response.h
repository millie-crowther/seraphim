#ifndef CALL_AND_RESPONSE_H
#define CALL_AND_RESPONSE_H

#include "metaphysics/substance.h"

static const uint32_t geometry_pool_size = 1000000;
static const uint32_t texture_pool_size = 1000000;

struct request_t {
    srph::f32vec3_t position;
    float radius;

    uint32_t hash;
    uint32_t _1;
    uint32_t substanceID;
    uint32_t status;

    uint32_t sdf_id;
    uint32_t material_id;
    uint64_t __unused;

    request_t();

    bool is_valid() const;
};

uint32_t request_geometry_index(const request_t *call);
uint32_t request_texture_index(const request_t *call);

struct request_pair_t {
    request_t geometry;
    request_t texture;
};


struct patch_t {
    uint32_t contents;
    uint32_t hash;
    float phi;
    uint32_t normal;
};

struct response_t {
    response_t(const request_t &call, substance_t *substance);

    patch_t patch;
    std::array<uint32_t, 8> normals;
    std::array<uint32_t, 8> colours;

    uint32_t squash(const srph::vec4_t &x) const;
};

void response_geometry(const request_t * request, const substance_t * substance, patch_t * patch);
void response_texture(const request_t * request, substance_t *substance, uint32_t * normals, uint32_t * colours);

#endif
