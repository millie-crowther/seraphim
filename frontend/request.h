#ifndef SERAPHIM_RENDER_REQUEST_H
#define SERAPHIM_RENDER_REQUEST_H

#include "../common/array.h"
#include "../backend/metaphysics.h"
#include "texture.h"

#include <threads.h>

static const uint32_t geometry_pool_size = 1000000;
static const uint32_t texture_pool_size = 1000000;
static const uint32_t number_of_requests = 2048;
static const uint32_t base_texture_binding = 11;

typedef enum texture_type_t {
    TEXTURE_TYPE_NORMAL = 0,
    TEXTURE_TYPE_COLOUR,
    TEXTURE_TYPE_PHYSICAL,
    TEXTURE_TYPE_MAXIMUM
} texture_type_t;

struct request_t {
    vec3f position;
    float radius;

    uint32_t hash;
    uint32_t sdf_id;
    uint32_t material_id;
    uint32_t status;
};

typedef struct request_handler_t {
    device_t * device;

    thrd_t thread;
    array_t(request_t *) request_queue;
    mtx_t request_mutex;
    mtx_t response_mutex;
    mtx_t cnd_mutex;
    cnd_t is_queue_empty;
    bool should_quit;

    texture_t textures[TEXTURE_TYPE_MAXIMUM];

    buffer_t patch_buffer;
    buffer_t request_buffer;
    buffer_t texture_hash_buffer;

    uint32_t patch_sample_size;
    uint32_t texture_size;

    sdf_t * sdfs;
    uint32_t * num_sdfs;

    material_t *materials;
    uint32_t *num_materials;
} request_handler_t;

void request_handler_create(request_handler_t *request_handler, uint32_t texture_size, uint32_t texture_depth,
                            uint32_t patch_sample_size, sdf_t *sdfs, uint32_t *num_sdfs, material_t *materials,
                            uint32_t *num_materials, device_t *device);
void request_handler_destroy(request_handler_t *request_handler);
void request_handler_handle_requests(request_handler_t * request_handler);
void request_handler_record_buffer_accesses(request_handler_t *request_handler, VkCommandBuffer command_buffer);

#endif
