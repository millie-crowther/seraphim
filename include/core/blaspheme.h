#ifndef BLASPHEME_H
#define BLASPHEME_H

#define BLASPHEME_DEBUG 1

#include "vk_mem_alloc.h"

#include <string>
#include <vector>
#include "render/renderer.h"
#include "ui/window.h"
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

	std::shared_ptr<device_t> device;

    // update functions
    void update_fps_counter(double delta);

    // debug fields
    bool is_debug;
    VkDebugReportCallbackEXT callback;

    std::shared_ptr<renderer_t> renderer;

    interval_revelator_t::follower_ptr_t frame_start_follower;

    std::shared_ptr<scheduler_t> scheduler;

	VmaAllocator allocator;
    VkInstance instance;
    VkSurfaceKHR surface;

    std::shared_ptr<window_t> window;
 
    std::shared_ptr<camera_t> test_camera;

public:
    blaspheme_t();
    ~blaspheme_t();

    void run();

    std::weak_ptr<renderer_t> get_renderer() const;
    std::weak_ptr<scheduler_t> get_scheduler() const;
    std::weak_ptr<window_t> get_window() const;
};

#endif
