#ifndef RENDERER_H
#define RENDERER_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <memory>

#include <chrono>
#include <list>
#include <map>
#include <set>

#include "core/buffer.h"
#include "core/command.h"
#include "metaphysics/substance.h"
#include "render/call_and_response.h"
#include "render/camera.h"
#include "render/light.h"
#include "render/swapchain.h"
#include "render/texture.h"
#include "ui/window.h"

struct push_constant_t {
    srph::u32vec2_t window_size;
    float render_distance;
    uint32_t current_frame;

    float phi_initial;
    float focal_depth;
    uint32_t number_of_calls;
    uint32_t texture_pool_size;

    float eye_transform[16];

    uint32_t texture_size;
    uint32_t texture_depth;
    uint32_t geometry_pool_size;
    float epsilon;
};

static const uint8_t frames_in_flight = 2;
static const uint32_t number_of_calls = 2048;
static const uint32_t geometry_pool_size = 1000000;
static const uint32_t texture_pool_size = 1000000;
static const uint32_t patch_sample_size = 2;
static const uint32_t max_cache_size = 1000;

struct renderer_t {
    std::set<uint32_t> indices;
    std::set<uint32_t> hashes;

    // fields
    srph::u32vec2_t work_group_count;
    srph::u32vec2_t work_group_size;
    uint32_t patch_image_size;
    push_constant_t push_constants;
    device_t *device;
    std::vector<VkFramebuffer> framebuffers;
    VkSurfaceKHR surface;
    VkRenderPass render_pass;

    VkPipeline graphics_pipeline;
    VkPipelineLayout pipeline_layout;
    std::vector<std::shared_ptr<srph::command_buffer_t>> command_buffers;

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

    char *fragment_shader_code;
    char *vertex_shader_code;

    substance_t *substances;
    uint32_t *num_substances;

    std::unique_ptr<srph::swapchain_t> swapchain;
    std::weak_ptr<srph::camera_t> main_camera;

    // textures
    std::unique_ptr<srph::texture_t> render_texture;
    std::unique_ptr<srph::texture_t> colour_texture;
    std::unique_ptr<srph::texture_t> normal_texture;

    // command pool
    std::unique_ptr<srph::command_pool_t> compute_command_pool;
    std::unique_ptr<srph::command_pool_t> graphics_command_pool;

    // buffers
    buffer_t patch_buffer;
    buffer_t substance_buffer;
    buffer_t call_buffer;
    buffer_t light_buffer;
    buffer_t pointer_buffer;
    buffer_t frustum_buffer;
    buffer_t lighting_buffer;

    std::map<srph::call_t, srph::response_t, srph::call_t::comparator_t>
        response_cache;
    std::__cxx11::list<std::map<srph::call_t, srph::response_t,
                                srph::call_t::comparator_t>::iterator>
        prev_calls;

    std::chrono::high_resolution_clock::time_point start;

    // initialisation functions
    VkShaderModule create_shader_module(std::string code);

    void create_render_pass();

    void create_graphics_pipeline();

    void create_compute_pipeline();

    void create_framebuffers();

    void create_command_buffers();

    void create_descriptor_set_layout();

    void create_descriptor_pool();

    void create_sync();

    void create_buffers();

    // helper functions
    void recreate_swapchain();

    void cleanup_swapchain();

    void handle_requests(uint32_t frame);

    void present(uint32_t image_index) const;

    srph::response_t get_response(const srph::call_t &call, substance_t *substance);

    // constructors and destructors
    renderer_t(device_t *device, substance_t *substances, uint32_t *num_substances,
               VkSurfaceKHR surface, srph::window_t *window,
               std::shared_ptr<srph::camera_t> test_camera,
               srph::u32vec2_t work_group_count, srph::u32vec2_t work_group_size,
               uint32_t max_image_size);

    ~renderer_t();

    // public functions
    void render();

    void set_main_camera(std::weak_ptr<srph::camera_t> camera);

    int get_frame_count();
};

#endif
