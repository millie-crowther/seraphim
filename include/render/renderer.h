#ifndef RENDERER_H
#define RENDERER_H

#include <memory>

#include "camera.h"
#include "core/buffer.h"
#include "render/octree.h"
#include "input/keyboard.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class renderer_t {
public:
    struct push_constant_t {
        u32vec2_t window_size;
        f32vec2_t dummy;         // alignment

        f32vec3_t camera_position;
        float dummy2;            // alignment

        f32vec3_t camera_right;
        float dummy3;
        f32vec3_t camera_up;
    };

private:
    push_constant_t push_constants;

    allocator_t allocator; // TODO : dont keep a copy

    // swapchain fields
    VkSwapchainKHR swapchain;
    std::vector<VkFramebuffer> swapchain_framebuffers;
    std::vector<VkImageView> swapchain_image_views;
    VkFormat swapchain_image_format;
    VkExtent2D swapchain_extents;

    VkSurfaceKHR surface;
    VkRenderPass render_pass;
    VkPipeline graphics_pipeline;
    VkDescriptorPool desc_pool;
    std::vector<VkDescriptorSet> desc_sets;

    // graphics pipeline fields
    VkDescriptorSetLayout descriptor_layout;
    VkPipelineLayout pipeline_layout;

    // synchronisation fields    
    static constexpr int frames_in_flight = 2;
    int current_frame;
    std::vector<VkSemaphore> image_available_semas;
    std::vector<VkSemaphore> render_finished_semas;
    std::vector<VkFence> in_flight_fences;

    // command pool fields
    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> command_buffers;
    std::vector<std::shared_ptr<buffer_t>> uniform_buffers;
    VkQueue graphics_queue;
    VkQueue present_queue;
    uint32_t graphics_family;
    uint32_t present_family;

    std::unique_ptr<buffer_t> vertex_buffer;

    VkShaderModule create_shader_module(std::string code, bool * success);

    void update_push_constants() const;

    bool create_swapchain();
    bool create_render_pass();
    bool create_graphics_pipeline();    
    // bool create_depth_resources();
    bool create_framebuffers();
    bool create_command_buffers();
    bool create_descriptor_set_layout();
    bool create_command_pool();
    bool create_descriptor_pool();
    bool create_sync();

    VkSurfaceFormatKHR select_surface_format(); 
    VkPresentModeKHR select_present_mode();
    VkExtent2D select_swap_extent();

    void recreate_swapchain();

    void cleanup_swapchain();

    bool init();

    static std::string vertex_shader_code;
    std::string fragment_shader_code;

    std::shared_ptr<sdf3_t> sphere;
    std::shared_ptr<sdf3_t> plane;
    std::vector<std::weak_ptr<sdf3_t>> renderable_sdfs;
    std::shared_ptr<octree_t> octree;

    std::weak_ptr<camera_t> main_camera;

public:
    // constructors and destructors
    renderer_t(
        const allocator_t & allocator,
        VkSurfaceKHR surface, uint32_t graphics_family, 
        uint32_t present_family, const u32vec2_t & window_size
    );
    ~renderer_t();


    void window_resize(const u32vec2_t & size);

    void render();

    void set_main_camera(std::weak_ptr<camera_t> camera);
};

#endif
