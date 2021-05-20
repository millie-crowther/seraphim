#ifndef SERAPHIM_H
#define SERAPHIM_H

#include "debug.h"

#include <condition_variable>
#include <set>
#include <string>
#include <vector>

#include "../backend/physics.h"
#include "../frontend/renderer.h"
#include "../frontend/window.h"

#define SERAPHIM_MAX_SUBSTANCES 100
#define SERAPHIM_MAX_SDFS 100
#define SERAPHIM_MAX_MATERIALS 100

struct seraphim_t {
#if SERAPHIM_DEBUG
    VkDebugReportCallbackEXT callback;
#endif
    device_t device;
    physics_t physics;
    VkInstance instance;
    VkSurfaceKHR surface;
    vec2u work_group_count;
    vec2u work_group_size;
    uint32_t num_substances;
    substance_t substances[SERAPHIM_MAX_SUBSTANCES];
    uint32_t num_sdfs;
    sdf_t sdfs[SERAPHIM_MAX_SDFS];
    uint32_t num_materials;
    material_t materials[SERAPHIM_MAX_MATERIALS];
    bool fps_monitor_quit;
    camera_t test_camera;
    window_t window;

    renderer_t renderer;
    std::thread fps_monitor_thread;
    std::condition_variable fps_cv;
};

substance_t *seraphim_create_substance(seraphim_t *srph, form_t *form, matter_t *matter);
sdf_t *seraphim_create_sdf(seraphim_t *srph, sdf_func_t phi, void *data);
material_t *seraphim_create_material(seraphim_t *srph, const vec3 * colour);

void seraphim_create(seraphim_t *seraphim, const char *title);
void seraphim_destroy(seraphim_t *engine);
void seraphim_run(seraphim_t *seraphim);

#endif
