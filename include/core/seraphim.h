#ifndef SERAPHIM_H
#define SERAPHIM_H

#include "core/debug.h"

#include <set>
#include <string>
#include <vector>

#include "physics/physics.h"
#include "render/renderer.h"
#include "ui/window.h"

namespace srph {
    class seraphim_t {
    private:
        // debug fields
    #if SERAPHIM_DEBUG
        VkDebugReportCallbackEXT callback;
        bool check_validation_layers();
        bool setup_debug_callback();
    #endif

        // initialisation functions
        void create_instance();
        std::vector<const char *> get_required_extensions();

        std::unique_ptr<device_t>   device;
        std::unique_ptr<renderer_t> renderer;
        std::unique_ptr<window_t>   window;
        std::unique_ptr<physics_t>  physics;

        VkInstance instance;
        VkSurfaceKHR surface;

        u32vec2_t work_group_count;
        u32vec2_t work_group_size;
     
        std::shared_ptr<camera_t> test_camera;      

        std::set<std::shared_ptr<substance_t>> substances;

        bool fps_monitor_quit;
        void monitor_fps();
        std::thread fps_monitor_thread;

    public:
        seraphim_t();
        ~seraphim_t();

        void run();

        renderer_t * get_renderer() const;
        window_t *   get_window()   const;

        void create(std::shared_ptr<substance_t> substance);
        void annihilate(std::shared_ptr<substance_t> substance);
    };
}

#endif
