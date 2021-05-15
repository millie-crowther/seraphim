#ifndef SERAPHIM_H
#define SERAPHIM_H

#include "core/debug.h"

#include <condition_variable>
#include <set>
#include <string>
#include <vector>

#include "physics/physics.h"
#include "render/renderer.h"
#include "ui/window.h"

#define SERAPHIM_MAX_SUBSTANCES 100
#define SERAPHIM_MAX_SDFS 100
#define SERAPHIM_MAX_MATERIALS 100

struct seraphim_t {
    // debug fields
#if SERAPHIM_DEBUG
    VkDebugReportCallbackEXT callback;

    bool check_validation_layers();

    bool setup_debug_callback();

#endif

    // initialisation functions
    void create_instance();

    std::vector<const char *> get_required_extensions();

    device_t device;
    std::unique_ptr<renderer_t> renderer;
    std::unique_ptr<window_t> window;
    srph_physics physics;

    VkInstance instance;
    VkSurfaceKHR surface;

    vec2u work_group_count;
    vec2u work_group_size;

    std::shared_ptr<camera_t> test_camera;

    uint32_t num_substances;
    substance_t substances[SERAPHIM_MAX_SUBSTANCES];

    uint32_t num_sdfs;
    sdf_t sdfs[SERAPHIM_MAX_SDFS];

    uint32_t num_materials;
    material_t materials[SERAPHIM_MAX_MATERIALS];

    bool fps_monitor_quit;

    void monitor_fps();

    std::thread fps_monitor_thread;
    std::condition_variable fps_cv;

    seraphim_t(const char *title);

    void run();
};

substance_t *seraphim_create_substance(seraphim_t *srph, form_t *form, matter_t *matter);
sdf_t *seraphim_create_sdf(seraphim_t *srph, sdf_func_t phi, void *data);
material_t *seraphim_create_material(seraphim_t *srph, const vec3 * colour);

void seraphim_destroy(seraphim_t *engine);

#endif
