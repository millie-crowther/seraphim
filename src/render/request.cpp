#include "render/request.h"

using namespace srph;

struct patch_t {
    uint32_t contents;
    uint32_t hash;
    float phi;
    uint32_t normal;
};

vec3_t vertices[8] = {vec3_t(0.0, 0.0, 0.0), vec3_t(2.0, 0.0, 0.0),
                      vec3_t(0.0, 2.0, 0.0), vec3_t(2.0, 2.0, 0.0),
                      vec3_t(0.0, 0.0, 2.0), vec3_t(2.0, 0.0, 2.0),
                      vec3_t(0.0, 2.0, 2.0), vec3_t(2.0, 2.0, 2.0)};

request_t null_requests[number_of_requests];
static const uint32_t null_status = 0;
static const uint32_t geometry_request = 1;
static const uint32_t texture_request = 2;


void response_geometry(const request_t *request, patch_t *patch, sdf_t *sdf);
void response_texture(const request_t *request, uint32_t *normals, uint32_t *colours, material_t *material, sdf_t *sdf);

static int request_handling_thread(void * request_handler);

static uint32_t squash(vec3 * x_){
    vec4 x;
    x.xyz = *x_;
    x.w = 0.0;

    uint8_t bytes[4];
    for (int i = 0; i < 4; i++) {
        bytes[i] = (uint8_t)(fmax(0.0, fmin(x.v[i] * 255.0, 255.0)));
    }
    return *(uint32_t *) bytes;
}

void response_geometry(const request_t *request, patch_t *patch, sdf_t *sdf) {
    bound3_t *bound = sdf_bound(sdf);
    vec3 midpoint;
    srph_bound3_midpoint(bound, &midpoint);
    vec3 position = {{request->position[0], request->position[1], request->position[2] }};
    vec3_subtract(&position, &position, &midpoint);

    uint32_t contains_mask = 0;

    for (int o = 0; o < 8; o++) {
        vec3_t d = vec3_t(position.x, position.y, position.z) + vertices[o] * request->radius;
        vec3 d1 = {{d[0], d[1], d[2]}};

        if (!sdf_contains(sdf, &d1)) {
            contains_mask |= 1 << o;
        }
    }

    vec3_t c = vec3_t(position.x, position.y, position.z) + request->radius;
    vec3 c1 = {{c[0], c[1], c[2]}};
    float phi = (float) sdf_distance(sdf, &c1);

    vec3 normal = sdf_normal(sdf, &c1);
    vec3_divide_f(&normal, &normal, 2);
    vec3_add_f(&normal, &normal, 0.5);
    uint32_t np = squash(&normal);

    uint32_t x_elem = contains_mask << 16;
    *patch = {x_elem, request->hash, phi, np};
}

void response_texture(const request_t *request, uint32_t *normals, uint32_t *colours, material_t *material, sdf_t *sdf) {
    bound3_t *bound = sdf_bound(sdf);
    vec3 midpoint;
    srph_bound3_midpoint(bound, &midpoint);
    vec3 position = {{request->position[0], request->position[1], request->position[2] }};
    vec3_subtract(&position, &position, &midpoint);

    for (int o = 0; o < 8; o++) {
        vec3_t d = vec3_t(position.x, position.y, position.z) + vertices[o] * request->radius;
        vec3 d1 = {{d[0], d[1], d[2]}};

        vec3 normal = sdf_normal(sdf, &d1);
        vec3_divide_f(&normal, &normal, 2);
        vec3_add_f(&normal, &normal, 0.5);
        normals[o] = squash(&normal);

        vec3 c;
        material_colour(material, NULL, &c);
        colours[o] = squash(&c);
    }
}

void request_handler_destroy(request_handler_t *request_handler) {
    request_handler->should_quit = true;
    cnd_broadcast(&request_handler->is_queue_empty);
    thrd_join(request_handler->thread, NULL);

    request_handler->normal_texture.reset();
    request_handler->colour_texture.reset();

    buffer_destroy(&request_handler->patch_buffer);
    buffer_destroy(&request_handler->request_buffer);
    buffer_destroy(&request_handler->texture_hash_buffer);
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

    request_handler->sdfs =sdfs;
    request_handler->num_sdfs = num_sdfs;
    request_handler->materials = materials;
    request_handler->num_materials = num_materials;
    request_handler->patch_sample_size = patch_sample_size;
    request_handler->texture_size = texture_size;

    srph_array_init(&request_handler->request_queue);
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

    u32vec3_t size_(size.x, size.y, size.z);

    request_handler->normal_texture = std::make_unique<texture_t>(
            11, request_handler->device, size_, VK_IMAGE_USAGE_SAMPLED_BIT,
            static_cast<VkFormatFeatureFlagBits>(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
                                                 VK_FORMAT_FEATURE_TRANSFER_DST_BIT),
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    request_handler->colour_texture = std::make_unique<texture_t>(
            12, request_handler->device, size_, VK_IMAGE_USAGE_SAMPLED_BIT,
            static_cast<VkFormatFeatureFlagBits>(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
                                                 VK_FORMAT_FEATURE_TRANSFER_DST_BIT),
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

static void handle_geometry_request(request_handler_t * request_handler, request_t * request){
    uint32_t sdf_id = request->sdf_id;
    if (sdf_id >= *request_handler->num_sdfs) {
        return;
    }

    patch_t patch{};
    response_geometry(request, &patch, &request_handler->sdfs[sdf_id]);
    uint32_t index = request->hash % geometry_pool_size;

    mtx_lock(&request_handler->response_mutex);
    request_handler->patch_buffer.write(&patch, 1, index);
    mtx_unlock(&request_handler->response_mutex);
}

static void handle_texture_request(request_handler_t * request_handler, request_t * request){
    uint32_t material_id = request->material_id;
    uint32_t sdf_id = request->sdf_id;
    if (material_id >= *request_handler->num_materials || sdf_id >= *request_handler->num_sdfs) {
        return;
    }

    uint32_t normals[8];
    uint32_t colours[8];
    response_texture(request, normals, colours, &request_handler->materials[material_id],
                     &request_handler->sdfs[sdf_id]);

    uint32_t index = request->hash % texture_pool_size;
    uint32_t texture_size = request_handler->texture_size;
    u32vec3_t p = u32vec3_t(
            index % texture_size,
            (index % (texture_size * texture_size)) / texture_size,
            index / texture_size / texture_size
    ) * request_handler->patch_sample_size;

    mtx_lock(&request_handler->response_mutex);
    request_handler->texture_hash_buffer.write(&request->hash, 1, index);
    request_handler->normal_texture->write(p, normals);
    request_handler->colour_texture->write(p, colours);
    mtx_unlock(&request_handler->response_mutex);
}

static int request_handling_thread(void * request_handler_){
    request_handler_t * request_handler = (request_handler_t *) request_handler_;
    while (!request_handler->should_quit){
        request_t * requests = NULL;

        mtx_lock(&request_handler->request_mutex);
        if (!srph_array_is_empty(&request_handler->request_queue)){
            requests = *request_handler->request_queue.first;
            srph_array_pop_front(&request_handler->request_queue);
        }
        mtx_unlock(&request_handler->request_mutex);

        if (requests == NULL) {
            cnd_wait(&request_handler->is_queue_empty, &request_handler->cnd_mutex);
        } else {
            for (size_t i = 0; i < number_of_requests; i++){
                request_t * request = &requests[i];
                if (request->status == geometry_request){
                    handle_geometry_request(request_handler, request);
                } else {
                    if (request->status == texture_request){
                        handle_texture_request(request_handler, request);
                    }
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

    void *memory_map = request_handler->request_buffer.map(0, number_of_requests);
    memcpy(requests, memory_map, number_of_requests * sizeof(request_t));
    memcpy(memory_map, null_requests, number_of_requests * sizeof(request_t));
    request_handler->request_buffer.unmap();

    mtx_lock(&request_handler->request_mutex);
    srph_array_push_back(&request_handler->request_queue);
    *(request_handler->request_queue.last) = requests;
    mtx_unlock(&request_handler->request_mutex);

    cnd_signal(&request_handler->is_queue_empty);
}

void request_handler_record_write(request_handler_t *request_handler, VkCommandBuffer command_buffer) {
    request_handler->request_buffer.record_read(command_buffer);

    mtx_lock(&request_handler->response_mutex);
    request_handler->patch_buffer.record_write(command_buffer);
    request_handler->texture_hash_buffer.record_write(command_buffer);
    request_handler->normal_texture->record_write(command_buffer);
    request_handler->colour_texture->record_write(command_buffer);
    mtx_unlock(&request_handler->response_mutex);
}
