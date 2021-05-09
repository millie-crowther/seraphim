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

    uint32_t number_of_requests;
    uint32_t patch_sample_size;
    uint32_t texture_size;

    sdf_t * sdfs;
    uint32_t * num_sdfs;

    material_t *materials;
    uint32_t *num_materials;
} request_handler_t;

void request_handler_create(request_handler_t *request_handler, uint32_t texture_size, uint32_t texture_depth,
                            uint32_t patch_sample_size, sdf_t *sdfs, uint32_t *num_sdfs, material_t *materials,
                            uint32_t *num_materials);
void request_handler_create_buffers(request_handler_t *request_handler, uint32_t number_of_requests, device_t *device);
void request_handler_destroy(request_handler_t *request_handler);
void request_handler_handle_requests(request_handler_t * request_handler);


struct patch_t {
    uint32_t contents;
    uint32_t hash;
    float phi;
    uint32_t normal;
};


#endif
