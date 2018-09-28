#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"
#include "buffer.h"
#include "chalet.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class renderer_t {
private:
    struct uniform_buffer_data_t {
        mat4_t model;
        mat4_t view;
        mat4_t proj;
    };

    camera_t * main_camera;

    // swapchain fields
    VkSwapchainKHR swapchain;
    std::vector<VkFramebuffer> swapchain_framebuffers;
    std::vector<image_t *> swapchain_images;
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
    std::vector<buffer_t *> uniform_buffers;
    VkQueue graphics_queue;
    VkQueue present_queue;
    uint32_t graphics_family;
    uint32_t present_family;

    image_t * depth_image;

    VkShaderModule create_shader_module(const std::vector<char>& code, bool * success);

    bool create_swapchain();
    bool create_render_pass();
    bool create_graphics_pipeline();    
    bool create_depth_resources();
    bool create_framebuffers();
    bool create_command_buffers(const std::shared_ptr<mesh_t>& mesh);
    void create_uniform_buffers();
    bool create_descriptor_set_layout();
    bool create_descriptor_sets();
    bool create_command_pool();
    bool create_descriptor_pool();
    bool create_sync();

    void update_descriptor_sets(texture_t * texture);

    VkSurfaceFormatKHR select_surface_format(); 
    VkPresentModeKHR select_present_mode();
    VkExtent2D select_swap_extent();

    void recreate_swapchain();

    void cleanup_swapchain();

public:
    // constructors and destructors
    renderer_t();
    ~renderer_t();

    bool init(
        VkSurfaceKHR surface, uint32_t graphics_fam, uint32_t present_fam, 
        VkExtent2D window_extents
    );

    void update_uniform_buffers(uint32_t image_index);

    void window_resize(int width, int height);

    // main method
    void render();
    
    chalet_t * chalet;

    // setters
    void set_main_camera(camera_t * camera);

    // accessors
    mat4_t get_view_matrix();
    mat4_t get_proj_matrix();

    void cleanup();
};

#endif
