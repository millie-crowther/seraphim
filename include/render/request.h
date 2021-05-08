#ifndef SERAPHIM_RENDER_REQUEST_H
#define SERAPHIM_RENDER_REQUEST_H

#include "core/array.h"
#include "metaphysics/substance.h"
#include "texture.h"

#include <threads.h>

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
bool request_is_geometry(const request_t *request);
bool request_is_texture(const request_t *request);

typedef struct request_handler_t {
    device_t * device;

    thrd_t thread;
    array_t(request_t) request_array;
    mtx_t mutex;
    cnd_t is_thread_empty;

    std::unique_ptr<texture_t> colour_texture;
    std::unique_ptr<texture_t> normal_texture;
    buffer_t patch_buffer;
    buffer_t request_buffer;
    buffer_t texture_hash_buffer;
} request_handler_t;

void request_handler_create(request_handler_t *request_handler, const vec3u *size);
void request_handler_create_buffers(request_handler_t *request_handler, uint32_t number_of_requests, device_t *device);
void request_handler_destroy(request_handler_t *request_handler);


struct patch_t {
    uint32_t contents;
    uint32_t hash;
    float phi;
    uint32_t normal;
};

void response_geometry(const request_t *request, patch_t *patch, sdf_t *sdf);
void response_texture(const request_t *request, uint32_t *normals, uint32_t *colours, material_t *material, sdf_t *sdf);

#endif
