#ifndef SERAPHIM_RENDER_REQUEST_H
#define SERAPHIM_RENDER_REQUEST_H

#include "metaphysics/substance.h"

static const uint32_t geometry_pool_size = 1000000;
static const uint32_t texture_pool_size = 1000000;

struct request_t {
    float position[3];
    float radius;

    uint32_t hash;
    uint32_t sdf_id;
    uint32_t material_id;
    uint32_t status;

    request_t();
};

uint32_t request_geometry_index(const request_t *call);
uint32_t request_texture_index(const request_t *call);
bool request_is_valid(const request_t *request);

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

void response_geometry(const request_t *request, patch_t *patch, sdf_t *sdf);
void response_texture(const request_t *request, uint32_t *normals, uint32_t *colours, material_t *material, sdf_t *sdf);

#endif
