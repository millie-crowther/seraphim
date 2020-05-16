#ifndef SERAPHIM_H
#define SERAPHIM_H

#include "core/debug.h"

#include <string>
#include <vector>
#include "render/renderer.h"
#include "ui/window.h"

class seraphim_t {
private:
    // initialisation functions
    void create_instance();
    std::vector<const char *> get_required_extensions();

	std::shared_ptr<device_t> device;

    // debug fields
#if SERAPHIM_DEBUG
    VkDebugReportCallbackEXT callback;
    bool check_validation_layers();
    bool setup_debug_callback();
#endif

    std::shared_ptr<renderer_t> renderer;

    VkInstance instance;
    VkSurfaceKHR surface;

    std::shared_ptr<window_t> window;
    u32vec2_t work_group_count;
    u32vec2_t work_group_size;
 
    std::shared_ptr<camera_t> test_camera;

public:
    seraphim_t();
    ~seraphim_t();

    void run();

    std::weak_ptr<renderer_t> get_renderer() const;
    std::weak_ptr<window_t> get_window() const;
};

#endif
