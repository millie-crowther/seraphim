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
    bool check_validation_layers(std::vector<const char *> validation_layers);
    std::vector<const char *> get_required_extensions();
    bool setup_debug_callback();
    bool is_suitable_device(VkPhysicalDevice device);
    VkPhysicalDevice select_device();

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
    GLFWwindow * window;

public:
    engine_t(bool is_debug);

    void run();
};

#endif
