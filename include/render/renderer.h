#ifndef RENDERER_H
#define RENDERER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>

#include "core/buffer.h"
#include "ui/window.h"
#include "render/octree.h"
#include "render/camera.h"
#include "render/swapchain.h"
#include "render/texture.h"
#include "core/command_buffer.h"

class renderer_t {
public:
    struct push_constant_t {
        u32vec2_t window_size;
        float render_distance;
        uint32_t current_frame;

        f32vec3_t camera_position;
        float dummy2;            // alignment

        f32vec3_t camera_right;
        float dummy3;

        f32vec3_t camera_up;
        float dummy4;
    };

private:
    // constants
    static constexpr uint8_t frames_in_flight = 2;

    // fields
    u32vec2_t work_group_count;
    u32vec2_t work_group_size;
    push_constant_t push_constants;
    VmaAllocator allocator;
    std::shared_ptr<device_t> device;
    std::vector<VkFramebuffer> framebuffers;
    VkSurfaceKHR surface;
    VkRenderPass render_pass;

    VkPipeline graphics_pipeline;
    VkPipelineLayout pipeline_layout;
    VkCommandPool command_pool;
    std::vector<std::unique_ptr<command_buffer_t>> command_buffers;

    VkPipeline compute_pipeline;
    VkPipelineLayout compute_pipeline_layout;
    VkCommandPool compute_command_pool;

    int current_frame;
    std::vector<VkSemaphore> image_available_semas;
    std::vector<VkSemaphore> compute_done_semas;
    std::vector<VkSemaphore> render_finished_semas;
    std::vector<VkFence> in_flight_fences;

    VkDescriptorSetLayout descriptor_layout;
    std::vector<VkDescriptorSet> desc_sets;
    VkDescriptorPool desc_pool;

    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue compute_queue;
    
    std::string fragment_shader_code;
    std::string vertex_shader_code;

    std::shared_ptr<sdf3_t> sphere;
    std::shared_ptr<sdf3_t> plane;
    std::vector<std::weak_ptr<sdf3_t>> renderable_sdfs;
    std::unique_ptr<octree_t> octree;
    std::unique_ptr<swapchain_t> swapchain;
    std::weak_ptr<camera_t> main_camera;
    std::unique_ptr<texture_t> render_texture;

    // private functions
    VkShaderModule create_shader_module(std::string code);
    bool create_render_pass();
    bool create_graphics_pipeline();    
    bool create_compute_pipeline();
    bool create_framebuffers();
    bool create_command_buffers();
    bool create_descriptor_set_layout();
    bool create_command_pool();
    bool create_descriptor_pool();
    bool create_sync();
    void cleanup_swapchain();
    void recreate_swapchain();
    bool init();

    void create_compute_command_buffers();

    uint32_t acquire_image() const;
    void present(uint32_t image_index) const;
    void submit_to_queue(VkQueue queue, VkCommandBuffer command_buffer, VkSemaphore wait_sema, VkSemaphore signal_sema, VkFence fence, VkPipelineStageFlags stage);

public:
    // constructors and destructors
    renderer_t(
        VmaAllocator allocator, std::shared_ptr<device_t> device,
        VkSurfaceKHR surface, std::shared_ptr<window_t> window,
        std::shared_ptr<camera_t> test_camera
    );
    ~renderer_t();

    // public functions
    void render();
    void set_main_camera(std::weak_ptr<camera_t> camera);
};

#endif
