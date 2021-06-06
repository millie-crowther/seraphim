#include "seraphim.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <string.h>

#include "../frontend/renderer.h"
#include <assert.h>
#include "file.h"

static bool check_validation_layers();
void create_instance(seraphim_t * seraphim);

#if SERAPHIM_DEBUG
const char * validation_layers[] = {
// "VK_LAYER_KHRONOS_validation"
};
const size_t num_validation_layers = sizeof(validation_layers) / sizeof(*validation_layers);
#endif

void seraphim_destroy(seraphim_t *engine) {
    engine->fps_monitor_quit = true;

    physics_destroy(&engine->physics);

    vkDeviceWaitIdle(engine->device.device);

    engine->fps_cv.notify_all();
    if (engine->fps_monitor_thread.joinable()) {
        engine->fps_monitor_thread.join();
    }

    renderer_destroy(&engine->renderer);

    device_destroy(&engine->device);

#if SERAPHIM_DEBUG
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
        engine->instance, "vkDestroyDebugReportCallbackEXT");

    if (func != NULL) {
        func(engine->instance, engine->callback, NULL);
    }
#endif

    vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);

    vkDestroyInstance(engine->instance, NULL);

    window_destroy(&engine->window);

    glfwTerminate();

    printf("Seraphim engine exiting gracefully.\n");
}

#if SERAPHIM_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT obj_type,
               uint64_t obj, size_t location, int32_t code, const char *layer_prefix,
               const char *msg, void *user_data) {
    printf("Validation layer debug message: %s\n", msg);
    return VK_FALSE;
}

static bool setup_debug_callback(seraphim_t * seraphim) {
    VkDebugReportCallbackCreateInfoEXT create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    create_info.flags =
            VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    create_info.pfnCallback = debug_callback;

    // load in function address, since its an extension
    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
            seraphim->instance, "vkCreateDebugReportCallbackEXT");

    return func != NULL &&
           func(seraphim->instance, &create_info, NULL, &seraphim->callback) == VK_SUCCESS;
}
#endif

substance_t *seraphim_create_substance(seraphim_t *srph, form_t *form,
                                       matter_t *matter) {
    assert(srph->num_substances < SERAPHIM_MAX_SUBSTANCES - 1);

    substance_t *new_substance = &srph->substances[srph->num_substances];
    substance_create(new_substance, form, matter, srph->num_substances);
    srph->num_substances++;
    return new_substance;
}

sdf_t *seraphim_create_sdf(seraphim_t *srph, sdf_func_t phi, void *data) {
    assert(srph->num_sdfs < SERAPHIM_MAX_SDFS - 1);

    sdf_t *new_sdf = &srph->sdfs[srph->num_sdfs];
    sdf_create(srph->num_sdfs, new_sdf, phi, data);
    srph->num_sdfs++;
    return new_sdf;
}

material_t *seraphim_create_material(seraphim_t *srph, const vec3 * colour) {
    assert(srph->num_materials < SERAPHIM_MAX_MATERIALS - 1);

    material_t *new_material = &srph->materials[srph->num_materials];
    material_create(new_material, srph->num_materials, colour);
    srph->num_materials++;
    return new_material;
}

void seraphim_create(seraphim_t *seraphim, const char *title) {
#if SERAPHIM_DEBUG
    printf("Running in debug mode\n");
#else
    printf("Running in release mode\n");
#endif

    seraphim->num_substances = 0;
    seraphim->num_sdfs = 0;
    seraphim->num_materials = 0;
    seraphim->work_group_count = {{48u, 20u}};
    seraphim->work_group_size = {{32u, 32u}};

    if (!glfwInit()) {
        PANIC("Error: Failed to initialise GLFW.");
    }

    vec2u window_size = {{
         seraphim->work_group_count.x * seraphim->work_group_size.x,
         seraphim->work_group_count.y * seraphim->work_group_size.y
    }};

    window_create(&seraphim->window, &window_size);
    window_set_title(&seraphim->window, title);

    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(NULL, &extension_count,
                                           extensions.data());

#if SERAPHIM_DEBUG
    printf("Available extensions:\n");
    for (uint32_t i = 0; i < extension_count; i++) {
        printf("\t%s\n", extensions[i].extensionName);
    }
#endif

    create_instance(seraphim);

#if SERAPHIM_DEBUG
    if (!setup_debug_callback(seraphim)) {
        PANIC("Error: Failed to setup debug callback.");
    }
#endif

    if (glfwCreateWindowSurface(seraphim->instance, seraphim->window.window, NULL, &seraphim->surface) !=
        VK_SUCCESS) {
        PANIC("Error: Failed to create window surface.");
    }

    device_create(&seraphim->device, seraphim->instance, seraphim->surface, validation_layers, num_validation_layers);

    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(seraphim->device.physical_device, &properties);
    uint32_t max_image_size = properties.limits.maxImageDimension3D;

#if SERAPHIM_DEBUG
    printf("Chosen physical device: %s\n", properties.deviceName);
    printf("\tMaximum storage buffer range: %u\n", properties.limits.maxStorageBufferRange);
    printf("\tMaximum shared memory size: %u\n", properties.limits.maxComputeSharedMemorySize);
    printf("\tMaximum 2d image size: %u\n", properties.limits.maxImageDimension2D);
    printf("\tMaximum 3d image size: %u\n", max_image_size);
#endif

    camera_create(&seraphim->test_camera);

    renderer_create(&seraphim->renderer,
        &seraphim->device, seraphim->substances, &seraphim->num_substances, seraphim->surface, &seraphim->window,
        &seraphim->test_camera, &seraphim->work_group_count, &seraphim->work_group_size, max_image_size, seraphim->materials, &seraphim->num_materials, seraphim->sdfs, &seraphim->num_sdfs);

    physics_create(&seraphim->physics, seraphim->substances, &seraphim->num_substances);
}

void monitor_fps(seraphim_t * seraphim) {
    int interval = 1; // seconds
    seraphim->fps_monitor_quit = false;

    std::mutex m;
    std::unique_lock<std::mutex> lock(m);

    while (!seraphim->fps_monitor_quit) {
        double physics_fps =
                (double) (seraphim->physics.get_frame_count()) / interval;
        double render_fps =
                (double) (seraphim->renderer.get_frame_count()) / interval;

        printf("Render: %f FPS; Physics: %f FPS\n", render_fps, physics_fps);
        seraphim->fps_cv.wait_for(lock, std::chrono::seconds(interval));
    }
}

static std::vector<const char *> get_required_extensions() {
    uint32_t extension_count = 0;
    const char **glfw_extensions =
        glfwGetRequiredInstanceExtensions(&extension_count);

    std::vector<const char *> required_extensions(glfw_extensions,
                                                  glfw_extensions + extension_count);

#if SERAPHIM_DEBUG
    required_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

    return required_extensions;
}

void create_instance(seraphim_t * seraphim) {
#if SERAPHIM_DEBUG
    if (!check_validation_layers()) {
        PANIC("Requested validation layers not available.");
    }
#endif

    // determine vulkan version
    typedef VkResult (*vulkan_version_func_t)(uint32_t *);
    auto vk_version_func = (vulkan_version_func_t)vkGetInstanceProcAddr(
            seraphim->instance, "vkEnumerateInstanceVersion");

    uint32_t version;
    unsigned major, minor, patch;
    if (vk_version_func != NULL && vk_version_func(&version) == VK_SUCCESS) {
        major = version >> 22;
        minor = (version - (major << 22)) >> 12;
        patch = version - (major << 22) - (minor << 12);
    } else {
        major = 1;
        minor = 0;
        patch = 0;
    }

    printf("Vulkan version: %u.%u.%u\n", major, minor, patch);

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = "Seraphim";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Seraphim";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    auto required_extensions = get_required_extensions();
    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = required_extensions.size();
    create_info.ppEnabledExtensionNames = required_extensions.data();

    printf("Enabled extensions: \n");
    for (uint32_t i = 0; i < create_info.enabledExtensionCount; i++) {
        printf("\t%s\n", create_info.ppEnabledExtensionNames[i]);
    }

#if SERAPHIM_DEBUG
    create_info.ppEnabledLayerNames = validation_layers;
    create_info.enabledLayerCount = num_validation_layers;

    printf("Enabled validation layers: \n");
    for (uint32_t i = 0; i < create_info.enabledLayerCount; i++) {
        printf("\t%s\n", create_info.ppEnabledLayerNames[i]);
    }
#else
    create_info.enabledLayerCount = 0;
#endif

    auto result = vkCreateInstance(&create_info, NULL, &seraphim->instance);
    if (result != VK_SUCCESS) {
        PANIC("Failed to create Vulkan instance!");
    }
}

static bool check_validation_layers() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);

    VkLayerProperties available_layers[layer_count];
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

    for (uint32_t i = 0; i < num_validation_layers; i++){
        bool layer_found = false;
        for (uint32_t j = 0; j < layer_count; j++){
            if (strcmp(validation_layers[i], available_layers[j].layerName) == 0){
                layer_found = true;
                break;
            }
        }

        if (!layer_found){
            return false;
        }
    }

    return true;
}

void seraphim_run(seraphim_t *seraphim) {
    physics_start(&seraphim->physics);

    seraphim->fps_monitor_thread = std::thread(monitor_fps, seraphim);

    window_show(&seraphim->window);

    uint32_t current_frame = 0;
    uint32_t frequency = 100;
    auto previous = std::chrono::steady_clock::now();
    double r_time;

    while (!window_should_close(&seraphim->window)) {
        glfwPollEvents();

        auto now = std::chrono::steady_clock::now();
        double delta =
            std::chrono::duration_cast<std::chrono::microseconds>(now - previous)
                .count() /
            1000000.0;
        r_time += 1.0 / delta;
        previous = now;

        mouse_update(&seraphim->window.mouse, delta);

        camera_update(&seraphim->test_camera, delta, seraphim->window.keyboard, seraphim->window.mouse);

        if (current_frame % frequency == frequency - 1) {
            r_time = 0;
        }

        seraphim->renderer.render();

        current_frame++;
    }
}
