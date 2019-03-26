#ifndef RENDERER_H
#define RENDERER_H

#include <memory>

#include "camera.h"
#include "buffer.h"
#include "input.h"
#include "render/image.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class renderer_t {
private:
    struct push_constant_t {
        uvec2_t window_size;
    } push_constants;

    VkPhysicalDevice physical_device;
    VkDevice device;

    // swapchain fields
    VkSwapchainKHR swapchain;
    std::vector<VkFramebuffer> swapchain_framebuffers;
    std::vector<std::unique_ptr<image_t>> swapchain_images;
    VkExtent2D swapchain_extents;
    VkExtent2D window_extents;

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
    std::vector<buffer_t> uniform_buffers;
    VkQueue graphics_queue;
    VkQueue present_queue;
    uint32_t graphics_family;
    uint32_t present_family;

    std::unique_ptr<image_t> depth_image;

    buffer_t vertex_buffer;

    VkShaderModule create_shader_module(std::string code, bool * success);

    bool create_swapchain();
    bool create_render_pass();
    bool create_graphics_pipeline();    
    bool create_depth_resources();
    bool create_framebuffers();
    bool create_command_buffers();
    bool create_descriptor_set_layout();
    bool create_descriptor_sets();
    bool create_command_pool();
    bool create_descriptor_pool();
    bool create_sync();

    VkSurfaceFormatKHR select_surface_format(); 
    VkPresentModeKHR select_present_mode();
    VkExtent2D select_swap_extent();

    void recreate_swapchain();

    void cleanup_swapchain();

    bool init();

public:
    // constructors and destructors
    renderer_t(
        VkPhysicalDevice physical_device, VkDevice device,
        VkSurfaceKHR surface, uint32_t graphics_family, 
        uint32_t present_family,VkExtent2D window_extents
    );
    ~renderer_t();


    void window_resize(uint32_t width, uint32_t height);

    // main method
    void render();
};

#endif
