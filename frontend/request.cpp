#include <cstring>
#include "request.h"

typedef struct patch_t {
    uint32_t contents;
    uint32_t hash;
    float phi;
    uint32_t normal;
} patch_t;

vec3 vertices[8] = {
    {{0.0, 0.0, 0.0}}, {{2.0, 0.0, 0.0}},
    {{0.0, 2.0, 0.0}}, {{2.0, 2.0, 0.0}},
    {{0.0, 0.0, 2.0}}, {{2.0, 0.0, 2.0}},
    {{0.0, 2.0, 2.0}}, {{2.0, 2.0, 2.0}}
};

request_t null_requests[number_of_requests];
static const uint32_t null_status = 0;
static const uint32_t geometry_request = 1;
static const uint32_t texture_request = 2;
static const uint32_t raycast_request = 3;

static int request_handling_thread(void * request_handler);

static uint32_t pack_vector(vec4 *x) {
    uint8_t bytes[4];
    for (int i = 0; i < 4; i++) {
        bytes[i] = (uint8_t)(fmax(0.0, fmin(x->v[i] * 255.0, 255.0)));
    }
    return *(uint32_t *) bytes;
}

void request_handler_destroy(request_handler_t *request_handler) {
    request_handler->should_quit = true;
    cnd_broadcast(&request_handler->is_queue_empty);
    thrd_join(request_handler->thread, NULL);

    for (int i = 0; i < TEXTURE_TYPE_MAXIMUM; i++){
        texture_destroy(&request_handler->textures[i]);
    }

    buffer_destroy(&request_handler->patch_buffer);
    buffer_destroy(&request_handler->request_buffer);
    buffer_destroy(&request_handler->texture_hash_buffer);
    buffer_destroy(&request_handler->raycast_buffer);
}

void request_handler_create(request_handler_t *request_handler, uint32_t texture_size, uint32_t texture_depth,
                            uint32_t patch_sample_size, sdf_t *sdfs, uint32_t *num_sdfs, material_t *materials,
                            uint32_t *num_materials, device_t *device) {
    request_handler->device = device;

    buffer_create(&request_handler->patch_buffer, 1, request_handler->device, geometry_pool_size, true,
                  sizeof(patch_t));
    buffer_create(&request_handler->request_buffer, 2, request_handler->device, number_of_requests, true, sizeof(request_t));
    buffer_create(&request_handler->texture_hash_buffer, 8, request_handler->device, texture_pool_size, true,
                  sizeof(uint32_t));
    buffer_create(&request_handler->raycast_buffer, 9, request_handler->device, number_of_raycasts, true, sizeof(intersection_t));

    request_handler->sdfs =sdfs;
    request_handler->num_sdfs = num_sdfs;
    request_handler->materials = materials;
    request_handler->num_materials = num_materials;
    request_handler->patch_sample_size = patch_sample_size;
    request_handler->texture_size = texture_size;

    array_create(&request_handler->request_queue);
    mtx_init(&request_handler->response_mutex, mtx_plain);
    mtx_init(&request_handler->request_mutex, mtx_plain);
    mtx_init(&request_handler->cnd_mutex, mtx_plain);
    cnd_init(&request_handler->is_queue_empty);
    request_handler->should_quit = false;
    thrd_create(&request_handler->thread, request_handling_thread, request_handler);

    for (size_t i = 0; i < number_of_requests; i++){
        null_requests[i].status = null_status;
    }

    vec3u size = {{ texture_size, texture_size, texture_depth }};
    vec3u_multiply_u(&size, &size, patch_sample_size);

    for (int i = 0; i < TEXTURE_TYPE_MAXIMUM; i++) {
        texture_create(&request_handler->textures[i], base_texture_binding + i, request_handler->device, &size,
                       VK_IMAGE_USAGE_SAMPLED_BIT,
                       (VkFormatFeatureFlagBits)(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
                                                            VK_FORMAT_FEATURE_TRANSFER_DST_BIT),
                       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    }
}

static void handle_geometry_request(request_handler_t * request_handler, request_t * request){
    uint32_t sdf_id = request->sdf_id;
    if (sdf_id >= *request_handler->num_sdfs) {
        return;
    }

    bound3_t *bound = sdf_bound(&request_handler->sdfs[sdf_id]);
    vec3 midpoint;
    bound3_midpoint(bound, &midpoint);
    vec3 position = {{request->position.x, request->position.y, request->position.z}};
    vec3_subtract(&position, &position, &midpoint);

    uint32_t containsMask = 0;

    for (int o = 0; o < 8; o++) {
        vec3 d;
        vec3_multiply_f(&d, &vertices[o], request->radius);
        vec3_add(&d, &d, &position);

        if (!sdf_contains(&request_handler->sdfs[sdf_id], &d)) {
            containsMask |= 1 << o;
        }
    }

    vec3 c;
    vec3_multiply_f(&c, &position, request->radius);
    float phi = (float) sdf_distance(&request_handler->sdfs[sdf_id], &c);

    vec4 normal = vec4_zero;
    normal.xyz = sdf_normal(&request_handler->sdfs[sdf_id], &c);
    vec4_divide_f(&normal, &normal, 2);
    vec4_add_f(&normal, &normal, 0.5);
    uint32_t packed_normal = pack_vector(&normal);

    patch_t patch = {
        .contents = containsMask << 16,
        .hash = request->hash,
        .phi = phi,
        .normal = packed_normal,
    };
    uint32_t index = request->hash % geometry_pool_size;

    mtx_lock(&request_handler->response_mutex);
    {
        buffer_write(&request_handler->patch_buffer, &patch, 1, index);
    }
    mtx_unlock(&request_handler->response_mutex);
}

static void handle_raycast_request(request_handler_t * request_handler, request_t * request) {
    intersection_t intersection = {
        .sdf_id = request->sdf_id,
    };

    if (intersection.sdf_id >= *request_handler->num_sdfs) {
        return;
    }


}

static void handle_texture_request(request_handler_t * request_handler, request_t * request){
    uint32_t material_id = request->material_id;
    uint32_t sdf_id = request->sdf_id;
    if (material_id >= *request_handler->num_materials || sdf_id >= *request_handler->num_sdfs) {
        return;
    }

    uint32_t normals[8];
    uint32_t colours[8];
    uint32_t physicals[8];
    bound3_t *bound = sdf_bound(&request_handler->sdfs[sdf_id]);
    vec3 midpoint;
    bound3_midpoint(bound, &midpoint);
    vec3 position = {{request->position.x, request->position.y, request->position.z}};
    vec3_subtract(&position, &position, &midpoint);

    for (int o = 0; o < 8; o++) {
        vec3 d;
        vec3_multiply_f(&d, &vertices[o], request->radius);
        vec3_add(&d, &d, &position);

        vec4 normal = vec4_zero;
        normal.xyz = sdf_normal(&request_handler->sdfs[sdf_id], &d);
        vec4_divide_f(&normal, &normal, 2);
        vec4_add_f(&normal, &normal, 0.5);
        normals[o] = pack_vector(&normal);

        vec4 colour = vec4_zero;
        material_colour(&request_handler->materials[material_id], NULL, &colour.xyz);
        colours[o] = pack_vector(&colour);

        vec4 physical;
        material_physical(&request_handler->materials[material_id], NULL, &physical);
        physicals[o] = pack_vector(&physical);
    }

    int index = (int) (request->hash % texture_pool_size);
    int texture_size = request_handler->texture_size;
    vec3i p = {{
        index % texture_size,
        (index % (texture_size * texture_size)) / texture_size,
        index / texture_size / texture_size
    }};
    vec3i_multiply_i(&p, &p, request_handler->patch_sample_size);

    mtx_lock(&request_handler->response_mutex);
    {
        buffer_write(&request_handler->texture_hash_buffer, &request->hash, 1, index);
        request_handler->textures[TEXTURE_TYPE_NORMAL].write(&p, normals);
        request_handler->textures[TEXTURE_TYPE_COLOUR].write(&p, colours);
        request_handler->textures[TEXTURE_TYPE_PHYSICAL].write(&p, physicals);
    }
    mtx_unlock(&request_handler->response_mutex);
}

static int request_handling_thread(void * request_handler_){
    request_handler_t * request_handler = (request_handler_t *) request_handler_;
    while (!request_handler->should_quit){
        request_t * requests = NULL;

        mtx_lock(&request_handler->request_mutex);
        {
            if (!array_is_empty(&request_handler->request_queue)) {
                requests = *request_handler->request_queue.first;
                array_pop_front(&request_handler->request_queue);
            }
        }
        mtx_unlock(&request_handler->request_mutex);

        if (requests == NULL) {
            cnd_wait(&request_handler->is_queue_empty, &request_handler->cnd_mutex);
        } else {
            for (size_t i = 0; i < number_of_requests; i++){
                request_t * request = &requests[i];
                if (request->status == geometry_request){
                    handle_geometry_request(request_handler, request);
                } else if (request->status == texture_request){
                    handle_texture_request(request_handler, request);
                } else if (request->status == raycast_request){
                    handle_raycast_request(request_handler, request);
                }
            }

            free(requests);
        }
    }

    return 0;
}

void request_handler_handle_requests(request_handler_t * request_handler) {
    vkDeviceWaitIdle(request_handler->device->device);

    request_t * requests = (request_t *) malloc(sizeof(request_t) * number_of_requests);

    void *memory_map = buffer_map(&request_handler->request_buffer, 0, number_of_requests);
    {
        memcpy(requests, memory_map, number_of_requests * sizeof(request_t));
        memcpy(memory_map, null_requests, number_of_requests * sizeof(request_t));
    }
    buffer_unmap(&request_handler->request_buffer);

    mtx_lock(&request_handler->request_mutex);
    {
        array_push_back(&request_handler->request_queue);
        *(request_handler->request_queue.last) = requests;
    }
    mtx_unlock(&request_handler->request_mutex);

    cnd_signal(&request_handler->is_queue_empty);
}

void request_handler_record_buffer_accesses(request_handler_t *request_handler, VkCommandBuffer command_buffer) {
    buffer_record_read(&request_handler->request_buffer, command_buffer);

    mtx_lock(&request_handler->response_mutex);
    {
        buffer_record_write(&request_handler->patch_buffer, command_buffer);
        buffer_record_write(&request_handler->texture_hash_buffer, command_buffer);
        for (int i = 0; i < TEXTURE_TYPE_MAXIMUM; i++) {
            request_handler->textures[i].record_write(command_buffer);
        }
    }
    mtx_unlock(&request_handler->response_mutex);
}
