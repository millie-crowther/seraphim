#ifndef BLASPHEME_H
#define BLASPHEME_H

#include "vk_mem_alloc.h"

#include <string>
#include <vector>
#include "render/renderer.h"
#include "input/keyboard.h"
#include "logic/scheduler.h"

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
    void update_fps_counter(double delta);

    // debug fields
    bool is_debug;
    VkDebugReportCallbackEXT callback;

    std::unique_ptr<renderer_t> renderer;

    scheduler_t scheduler;

    allocator_t allocator;

    VkInstance instance;
    VkSurfaceKHR surface;

	keyboard_t keyboard;
 
    GLFWwindow * window;

public:
    blaspheme_t(bool is_debug);
    ~blaspheme_t();

    void run();
	
    void window_resize(const u32vec2_t & size);
	void keyboard_event(int key, int action, int mods);
};

#endif
