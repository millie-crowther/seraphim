#ifndef ENGINE_H
#define ENGINE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

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
    bool is_suitable_device(VkPhysicalDevice physical_device);
    bool device_has_extension(VkPhysicalDevice physical_device, const char * extension);
    bool has_adequate_swapchain(VkPhysicalDevice physical_device);
    int get_graphics_queue_family(VkPhysicalDevice physical_device);
    int get_present_queue_family(VkPhysicalDevice physical_device);
    
    bool create_logical_device(VkPhysicalDevice physical_device);

    // swap chain creation
    bool create_swapchain(VkPhysicalDevice physical_device);
    bool create_image_views();
    VkSurfaceFormatKHR select_surface_format(VkPhysicalDevice physical_device); 
    VkPresentModeKHR select_present_mode(VkPhysicalDevice physical_device);
    VkExtent2D select_swap_extent(VkPhysicalDevice physical_device);

    // graphics pipeline
    bool create_graphics_pipeline();    
    std::vector<char> load_file(std::string filename);
    VkShaderModule create_shader_module(const std::vector<char>& code, bool * success);

    // update functions
    bool should_quit(); 
    void update();

    // cleanup functions
    void cleanup();

    // debug fields
    bool is_debug;
    VkDebugReportCallbackEXT callback;

    // main fields
    VkInstance instance;
    VkDevice device;
    VkSurfaceKHR surface;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkPipelineLayout pipeline_layout;

    // swapchain fields
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchain_images;
    VkFormat swapchain_image_format;
    VkExtent2D swapchain_extents;
    std::vector<VkImageView> swapchain_image_views;
     
    int width;
    int height;
    GLFWwindow * window;

public:
    engine_t(bool is_debug);

    void run();
};

#endif
