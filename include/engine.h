#ifndef ENGINE_H
#define ENGINE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

class engine_t {
private:
    // initialisation functions
    void init();
    void create_instance();
    bool check_validation_layers();
    std::vector<const char *> get_required_extensions();
    bool setup_debug_callback();
    bool is_suitable_device(VkPhysicalDevice physical_device);
    VkPhysicalDevice select_device();
    int get_graphics_queue_family(VkPhysicalDevice physical_device);
    int get_present_queue_family(VkPhysicalDevice physical_device);
    bool create_logical_device(VkPhysicalDevice physical_device);

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
    GLFWwindow * window;

public:
    engine_t(bool is_debug);

    void run();
};

#endif
