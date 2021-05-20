#ifndef RENDERER_H
#define RENDERER_H

#include <memory>

#include <chrono>
#include <list>
#include <map>
#include <set>

#include "request.h"
#include "buffer.h"
#include "command.h"
#include "../backend/metaphysics.h"
#include "request.h"
#include "../common/camera.h"
#include "../common/light.h"
#include "swapchain.h"
#include "texture.h"
#include "ui.h"
#include "texture.h"
#include "shader.h"
#include "swapchain.h"
#include "../common/camera.h"

struct push_constant_t {
    vec2u window_size;
    float render_distance;
    uint32_t current_frame;

    float phi_initial;
    float focal_depth;
    uint32_t number_of_requests;
    uint32_t texture_pool_size;

    float eye_transform[16];

    uint32_t texture_size;
    uint32_t texture_depth;
    uint32_t geometry_pool_size;
    float epsilon;
};

static const uint8_t frames_in_flight = 2;
static const uint32_t patch_sample_size = 2;

struct renderer_t {
    vec2u work_group_count;
    vec2u work_group_size;
    uint32_t texture_size;
    push_constant_t push_constants;
    device_t *device;
    std::vector<VkFramebuffer> framebuffers;
    VkSurfaceKHR surface;
    VkRenderPass render_pass;
    VkPipeline graphics_pipeline;
    VkPipelineLayout pipeline_layout;
    command_buffer_t * command_buffers;
    VkPipeline compute_pipeline;
    VkPipelineLayout compute_pipeline_layout;
    int frames;
    int current_frame;

    VkSemaphore image_available_semas[frames_in_flight];
    VkSemaphore compute_done_semas[frames_in_flight];
    VkSemaphore render_finished_semas[frames_in_flight];
    VkFence in_flight_fences[frames_in_flight];

    VkDescriptorSetLayout descriptor_layout;
    std::vector<VkDescriptorSet> desc_sets;
    VkDescriptorPool desc_pool;

    VkQueue present_queue;

    shader_t fragment_shader;
    shader_t vertex_shader;

    substance_t *substances;
    uint32_t *num_substances;
    camera_t * main_camera;
    texture_t render_texture;

    // command pool
    command_pool_t compute_command_pool;
    command_pool_t graphics_command_pool;

    // buffers
    buffer_t substance_buffer;
    buffer_t light_buffer;
    buffer_t pointer_buffer;
    buffer_t frustum_buffer;
    buffer_t lighting_buffer;

    request_handler_t request_handler;

    std::unique_ptr<swapchain_t> swapchain;
    std::chrono::high_resolution_clock::time_point start;

    void create_render_pass();
    void create_graphics_pipeline();
    void create_compute_pipeline();
    void create_framebuffers();
    void create_command_buffers();
    void create_descriptor_pool();
    void create_sync();
    void create_buffers();
    void recreate_swapchain();
    void cleanup_swapchain();
    void render();

    int get_frame_count();
};

void renderer_create(renderer_t * renderer, device_t *device, substance_t *substances, uint32_t *num_substances, VkSurfaceKHR surface,
        window_t *window, camera_t *test_camera, vec2u *work_group_count,
vec2u *work_group_size, uint32_t max_image_size, material_t *materials, uint32_t *num_materials,
sdf_t *sdfs, uint32_t *num_sdfs);
void renderer_destroy(renderer_t * renderer);

#endif
