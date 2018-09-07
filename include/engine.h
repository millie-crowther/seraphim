#ifndef ENGINE_H
#define ENGINE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vk_mem_alloc.h"

#include <string>
#include <vector>

#include "texture.h"
#include "mesh.h"

class engine_t {
private:
    /*
      initialisation functions
    */
    void init();
    void create_instance();
    bool check_validation_layers();
    std::vector<const char *> get_required_extensions();
    bool setup_debug_callback();

    // physical device selection
    VkPhysicalDevice select_device();
    bool is_suitable_device(VkPhysicalDevice phys_device);
    bool device_has_extension(VkPhysicalDevice phys_device, const char * extension);
    bool has_adequate_swapchain(VkPhysicalDevice phys_device);
    int get_graphics_queue_family(VkPhysicalDevice phys_device);
    int get_present_queue_family(VkPhysicalDevice phys_device);
    
    bool create_logical_device();

    // swap chain management
    bool create_swapchain();
    void recreate_swapchain();
    void cleanup_swapchain();
    VkSurfaceFormatKHR select_surface_format(); 
    VkPresentModeKHR select_present_mode();
    VkExtent2D select_swap_extent();

    bool create_descriptor_set_layout();
    bool create_descriptor_pool();
    bool create_descriptor_sets();

    bool create_render_pass();
    bool create_framebuffers();
    bool create_command_pool();
    bool create_command_buffers();
    bool create_sync();

    bool create_depth_resources();

    // graphics pipeline
    bool create_graphics_pipeline();    
    std::vector<char> load_file(std::string filename);
    VkShaderModule create_shader_module(const std::vector<char>& code, bool * success);

    // update functions
    bool should_quit();
    void render(int current_frame); 
    void update();
    void update_uniform_buffers(uint32_t image_index);

    // cleanup functions
    void cleanup();

    // debug fields
    bool is_debug;
    VkDebugReportCallbackEXT callback;

    // main fields
    VmaAllocator allocator;

    // buffers
    mesh_t * mesh;
    std::vector<buffer_t *> uniform_buffers;

    // depth buffer
    image_t * depth_image;

    VkPhysicalDevice physical_device;
    VkInstance instance;
    VkDevice device;
    VkSurfaceKHR surface;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkDescriptorPool desc_pool;
    std::vector<VkDescriptorSet> desc_sets;

    // graphics pipeline fields
    VkPipeline graphics_pipeline;
    VkDescriptorSetLayout descriptor_layout;
    VkPipelineLayout pipeline_layout;
    VkRenderPass render_pass;

    // command pool fields
    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> command_buffers;

    // swapchain fields
    std::vector<image_t *> swapchain_images;
    VkSwapchainKHR swapchain;
    VkExtent2D swapchain_extents;
    std::vector<VkFramebuffer> swapchain_framebuffers;
    
    // synchronisation fields
    static constexpr int frames_in_flight = 2;
    std::vector<VkSemaphore> image_available_semas;
    std::vector<VkSemaphore> render_finished_semas;
    std::vector<VkFence> in_flight_fences;
 
    GLFWwindow * window;

public:
    engine_t(bool is_debug);

    void window_resize(int w, int h);

    void run();
};

#endif
