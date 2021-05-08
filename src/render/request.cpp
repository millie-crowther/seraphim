#include "render/request.h"

using namespace srph;

vec3_t vertices[8] = {vec3_t(0.0, 0.0, 0.0), vec3_t(2.0, 0.0, 0.0),
                      vec3_t(0.0, 2.0, 0.0), vec3_t(2.0, 2.0, 0.0),
                      vec3_t(0.0, 0.0, 2.0), vec3_t(2.0, 0.0, 2.0),
                      vec3_t(0.0, 2.0, 2.0), vec3_t(2.0, 2.0, 2.0)};

static const uint32_t null_status = 0;
static const uint32_t geometry_request = 1;
static const uint32_t texture_request = 2;

request_t::request_t() {
    status = null_status;
}

uint32_t request_geometry_index(const request_t *call) {
    return call->hash % geometry_pool_size;
}

uint32_t request_texture_index(const request_t *call) {
    return call->hash % texture_pool_size;
}

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

void
response_texture(const request_t *request, uint32_t *normals, uint32_t *colours, material_t *material, sdf_t *sdf) {
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

bool request_is_geometry(const request_t *request) {
    return request->status == geometry_request;
}

bool request_is_texture(const request_t *request) {
    return request->status == texture_request;
}

void request_handler_destroy(request_handler_t *request_handler) {
    request_handler->normal_texture.reset();
    request_handler->colour_texture.reset();

    buffer_destroy(&request_handler->patch_buffer);
    buffer_destroy(&request_handler->request_buffer);
    buffer_destroy(&request_handler->texture_hash_buffer);
}

void request_handler_create(request_handler_t *request_handler, uint32_t texture_size, uint32_t texture_depth,
                            uint32_t patch_sample_size, sdf_t *sdfs, uint32_t *num_sdfs, material_t *materials,
                            uint32_t *num_materials) {
    request_handler->sdfs =sdfs;
    request_handler->num_sdfs = num_sdfs;
    request_handler->materials = materials;
    request_handler->num_materials = num_materials;
    request_handler->patch_sample_size = patch_sample_size;
    request_handler->texture_size = texture_size;

    vec3u size = {{
          texture_size * patch_sample_size,
          texture_size * patch_sample_size,
          texture_depth * patch_sample_size
    }};

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

void request_handler_create_buffers(request_handler_t *request_handler, uint32_t number_of_requests, device_t *device) {
    request_handler->device = device;
    request_handler->number_of_requests = number_of_requests;

    buffer_create(&request_handler->patch_buffer, 1, request_handler->device, geometry_pool_size, true,
                  sizeof(patch_t));
    buffer_create(&request_handler->request_buffer, 2, request_handler->device, number_of_requests, true, sizeof(request_t));
    buffer_create(&request_handler->texture_hash_buffer, 8, request_handler->device, texture_pool_size, true,
                  sizeof(uint32_t));
}


static void handle_geometry_request(request_handler_t * request_handler, request_t * request){
    uint32_t sdf_id = request->sdf_id;
    if (sdf_id >= *request_handler->num_sdfs) {
        return;
    }

    patch_t patch{};
    response_geometry(request, &patch, &request_handler->sdfs[sdf_id]);
    uint32_t index = request_geometry_index(request);
    request_handler->patch_buffer.write(&patch, 1, index);
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

    uint32_t index = request_texture_index(request);
    uint32_t texture_size = request_handler->texture_size;
    u32vec3_t p = u32vec3_t(
            index % texture_size,
            (index % (texture_size * texture_size)) / texture_size,
            index / texture_size / texture_size
    ) * request_handler->patch_sample_size;

    request_handler->texture_hash_buffer.write(&request->hash, 1, index);

    request_handler->normal_texture->write(p, normals);
    request_handler->colour_texture->write(p, colours);
}

void request_handler_handle_requests(request_handler_t * request_handler) {
    vkDeviceWaitIdle(request_handler->device->device);

    std::vector<request_t> requests(request_handler->number_of_requests);
    std::vector<request_t> null_requests(request_handler->number_of_requests);

    void *memory_map = request_handler->request_buffer.map(0, requests.size());
    memcpy(requests.data(), memory_map, requests.size() * sizeof(request_t));
    memcpy(memory_map, null_requests.data(), requests.size() * sizeof(request_t));
    request_handler->request_buffer.unmap();

    for (size_t i = 0; i < requests.size(); i++){
        request_t * request = &requests[i];
        if (request_is_geometry(request)){
            handle_geometry_request(request_handler, request);
        } else if (request_is_texture(request)){
            handle_texture_request(request_handler, request);
        }
    }
}