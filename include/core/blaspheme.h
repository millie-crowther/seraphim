#ifndef BLASPHEME_H
#define BLASPHEME_H

#include "vk_mem_alloc.h"

#include <string>
#include <vector>
#include "render/renderer.h"
#include "input/keyboard.h"

class blaspheme_t {
private:
    /*
      initialisation functions
    */
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

    // update functions
    bool should_quit();

    // debug fields
    bool is_debug;
    VkDebugReportCallbackEXT callback;

    VmaAllocator allocator;

    std::unique_ptr<renderer_t> renderer;

    VkInstance instance;
    VkSurfaceKHR surface;

	keyboard_t keyboard;
 
    GLFWwindow * window;

    // static fields
    static VkPhysicalDevice physical_device;
    static VkDevice device;

public:
    blaspheme_t(bool is_debug);
    ~blaspheme_t();

    void run();
	
    void window_resize(uint32_t width, uint32_t height);
	void keyboard_event(int key, int action, int mods);

    // static getters
    static VkDevice get_device();
    static VkPhysicalDevice get_physical_device();
};

#endif
