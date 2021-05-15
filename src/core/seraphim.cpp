#include "core/seraphim.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "render/renderer.h"
#include <assert.h>
#include <ui/file.h>

const std::vector<const char *> validation_layers = {
#if SERAPHIM_DEBUG
// "VK_LAYER_KHRONOS_validation"
#endif
};

void seraphim_destroy(seraphim_t *engine) {
    engine->fps_monitor_quit = true;

    physics_destroy(&engine->physics);

    vkDeviceWaitIdle(engine->device.device);

    engine->fps_cv.notify_all();
    if (engine->fps_monitor_thread.joinable()) {
        engine->fps_monitor_thread.join();
    }
    // delete renderer early to release resources at appropriate time
    engine->renderer.reset();

    // destroy device
    device_destroy(&engine->device);

    // destroy debug callback
#if SERAPHIM_DEBUG
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
        engine->instance, "vkDestroyDebugReportCallbackEXT");

    if (func != NULL) {
        func(engine->instance, engine->callback, NULL);
    }
#endif

    vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);

    // destroy instance
    vkDestroyInstance(engine->instance, NULL);

    engine->window.reset();

    glfwTerminate();

    printf("Seraphim engine exiting gracefully.\n");
}

#if SERAPHIM_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT obj_type,
               uint64_t obj, size_t location, int32_t code, const char *layer_prefix,
               const char *msg, void *user_data) {
    std::cout << "Validation layer debug message: " << msg << std::endl;
    return VK_FALSE;
}

#endif

substance_t *seraphim_create_substance(seraphim_t *srph, form_t *form,
                                       matter_t *matter) {
    assert(srph->num_substances < SERAPHIM_MAX_SUBSTANCES - 1);

    substance_t *new_substance = &srph->substances[srph->num_substances];
    *new_substance = substance_t(form, matter, srph->num_substances);
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

seraphim_t::seraphim_t(const char *title) {
#if SERAPHIM_DEBUG
    std::cout << "Running in debug mode." << std::endl;
#else
    std::cout << "Running in release mode." << std::endl;
#endif

    num_substances = 0;
    num_sdfs = 0;
    num_materials = 0;
    work_group_count = {{48u, 20u}};
    work_group_size = {{32u, 32u}};

    if (!glfwInit()) {
        throw std::runtime_error("Error: Failed to initialise GLFW.");
    }

    vec2u window_size = {{
        work_group_count.x * work_group_size.x,
        work_group_count.y * work_group_size.y
    }};
    window = std::make_unique<window_t>(&window_size);

    window_set_title(window.get(), title);

    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, NULL);
    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count,
                                           extensions.data());

    std::cout << "Available extensions:" << std::endl;
    for (const auto &extension : extensions) {
        std::cout << "\t" << extension.extensionName << std::endl;
    }

    create_instance();

#if SERAPHIM_DEBUG
    if (!setup_debug_callback()) {
        throw std::runtime_error("Error: Failed to setup debug callback.");
    }
#endif

    if (glfwCreateWindowSurface(instance, window->window, NULL, &surface) !=
        VK_SUCCESS) {
        throw std::runtime_error("Error: Failed to create window surface.");
    }

    device_create(&device, instance, surface, validation_layers);

#if SERAPHIM_DEBUG
    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(device.physical_device, &properties);
    std::cout << "Chosen physical device: " << properties.deviceName << std::endl;
    std::cout << "\tMaximum storage buffer range: "
              << properties.limits.maxStorageBufferRange << std::endl;
    std::cout << "\tMaximum shared memory  size: "
              << properties.limits.maxComputeSharedMemorySize << std::endl;
    std::cout << "\tMaximum 2d image size: " << properties.limits.maxImageDimension2D
              << std::endl;

    uint32_t max_image_size = properties.limits.maxImageDimension3D;
    std::cout << "\tMaximum 3d image size: " << max_image_size << std::endl;
#endif

    test_camera = std::make_shared<camera_t>();

    renderer = std::make_unique<renderer_t>(
        &device, substances, &num_substances, surface, window.get(),
        test_camera, &work_group_count, &work_group_size, max_image_size, materials, &num_materials, sdfs, &num_sdfs);

    physics_create(&physics, substances, &num_substances);
}

void seraphim_t::monitor_fps() {
    int interval = 1; // seconds
    fps_monitor_quit = false;

    std::mutex m;
    std::unique_lock<std::mutex> lock(m);

    while (!fps_monitor_quit) {
        double physics_fps =
            static_cast<double>(physics.get_frame_count()) / interval;
        double render_fps =
            static_cast<double>(renderer->get_frame_count()) / interval;

        std::cout << "Render: " << render_fps << " FPS; "
                  << "Physics: " << physics_fps << " FPS" << std::endl;
        fps_cv.wait_for(lock, std::chrono::seconds(interval));
    }
}

std::vector<const char *> seraphim_t::get_required_extensions() {
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

void seraphim_t::create_instance() {
#if SERAPHIM_DEBUG
    if (!check_validation_layers()) {
        throw std::runtime_error("Requested validation layers not available.");
    }
#endif

    // determine vulkan version
    typedef VkResult (*vulkan_version_func_t)(uint32_t *);
    auto vk_version_func = (vulkan_version_func_t)vkGetInstanceProcAddr(
        instance, "vkEnumerateInstanceVersion");

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

    std::cout << "Vulkan version: " << major << '.' << minor << '.' << patch
              << std::endl;

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

    std::cout << "Enabled extensions: " << std::endl;
    for (uint32_t i = 0; i < create_info.enabledExtensionCount; i++) {
        std::cout << "\t" << create_info.ppEnabledExtensionNames[i] << std::endl;
    }

#if SERAPHIM_DEBUG
    create_info.ppEnabledLayerNames = validation_layers.data();
    create_info.enabledLayerCount = validation_layers.size();
#else
    create_info.enabledLayerCount = 0;
#endif

    std::cout << "Enabled validation layers: " << std::endl;
    for (uint32_t i = 0; i < create_info.enabledLayerCount; i++) {
        std::cout << "\t" << create_info.ppEnabledLayerNames[i] << std::endl;
    }

    auto result = vkCreateInstance(&create_info, NULL, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance!");
    }
}

bool seraphim_t::setup_debug_callback() {
    VkDebugReportCallbackCreateInfoEXT create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    create_info.flags =
        VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    create_info.pfnCallback = debug_callback;

    // load in function address, since its an extension
    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugReportCallbackEXT");

    return func != NULL &&
           func(instance, &create_info, NULL, &callback) == VK_SUCCESS;
}

bool seraphim_t::check_validation_layers() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    return std::all_of(validation_layers.begin(), validation_layers.end(),
                       [available_layers](auto &layer) {
                           return std::any_of(
                               available_layers.begin(), available_layers.end(),
                               [layer](auto &properties) {
                                   return layer == std::string(properties.layerName);
                               });
                       });
}

void seraphim_t::run() {
    physics_start(&physics);

    fps_monitor_thread = std::thread(&seraphim_t::monitor_fps, this);

    window->show();

    uint32_t current_frame = 0;
    uint32_t frequency = 100;
    auto previous = std::chrono::steady_clock::now();
    double r_time;

    while (!window->should_close()) {
        glfwPollEvents();

        auto now = std::chrono::steady_clock::now();
        double delta =
            std::chrono::duration_cast<std::chrono::microseconds>(now - previous)
                .count() /
            1000000.0;
        r_time += 1.0 / delta;
        previous = now;

        mouse_update(&window->mouse, delta);

        test_camera->update(delta, *window->keyboard, window->mouse);

        if (current_frame % frequency == frequency - 1) {
            r_time = 0;
        }

        renderer->render();

        current_frame++;
    }
}
