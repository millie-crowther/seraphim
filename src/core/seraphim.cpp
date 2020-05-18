#include "core/seraphim.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <set>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <memory>
#include "render/renderer.h"

#include "logic/scheduler.h"

#if SERAPHIM_DEBUG
const std::vector<const char *> validation_layers = {
    "VK_LAYER_LUNARG_standard_validation"
};
#endif

seraphim_t::seraphim_t(){
#if SERAPHIM_DEBUG
    std::cout << "Running in debug mode." << std::endl;
#else 
    std::cout << "Running in release mode." << std::endl;
#endif

    work_group_count = u32vec2_t(14);
    work_group_size = u32vec2_t(32);

    if (!glfwInit()){
        throw std::runtime_error("Error: Failed to initialise GLFW.");
    }

    window = std::make_shared<window_t>(work_group_count.hadamard(work_group_size) * 2);

    create_instance();

    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

    std::cout << "Available extensions:" << std::endl;
    for (const auto & extension : extensions) {
	    std::cout << "\t" << extension.extensionName << std::endl;
    }

#if SERAPHIM_DEBUG
    if (!setup_debug_callback()){
        throw std::runtime_error("Error: Failed to setup debug callback.");
    }
#endif

    if (glfwCreateWindowSurface(instance, window->get_window(), nullptr, &surface) != VK_SUCCESS) {
	    throw std::runtime_error("Error: Failed to create window surface.");
    }

#if SERAPHIM_DEBUG
    device = std::make_shared<device_t>(instance, surface, validation_layers);

    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(device->get_physical_device(), &properties);
    std::cout << "Chosen physical device: " << properties.deviceName << std::endl;
    std::cout << "\tMaximum storage buffer range: " << properties.limits.maxStorageBufferRange << std::endl;
    std::cout << "\tMaximum shared memory  size: " << properties.limits.maxComputeSharedMemorySize << std::endl;
    std::cout << "\tMaximum image size: " << properties.limits.maxImageDimension3D << std::endl;
#else   
    device = std::make_shared<device_t>(instance, surface, std::vector<const char *>());
#endif

    test_camera = std::make_shared<camera_t>();

    renderer = std::make_shared<renderer_t>(
        device, surface, window, test_camera, work_group_count, work_group_size
    );
}

seraphim_t::~seraphim_t(){
    vkDeviceWaitIdle(device->get_device());

    // delete renderer early to release resources at appropriate time
    renderer.reset();

    // destroy device
    device.reset();

    // destroy debug callback
#if SERAPHIM_DEBUG
    auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(
        instance, "vkDestroyDebugReportCallbackEXT"
    );

    if (func != nullptr){
        func(instance, callback, nullptr);
    }
#endif
   
    vkDestroySurfaceKHR(instance, surface, nullptr);

    // destroy instance
    vkDestroyInstance(instance, nullptr);

    window.reset();

    glfwTerminate();

    std::cout << "Seraphim engine exiting gracefully." << std::endl;
}

std::weak_ptr<renderer_t> 
seraphim_t::get_renderer() const {
    return renderer;
}

std::vector<const char *>
seraphim_t::get_required_extensions(){
    uint32_t      extension_count = 0;
    const char ** glfw_extensions = glfwGetRequiredInstanceExtensions(&extension_count);

    std::vector<const char *> required_extensions(glfw_extensions, glfw_extensions + extension_count);

#if SERAPHIM_DEBUG
    required_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

    return required_extensions;
}

void
seraphim_t::create_instance(){
#if SERAPHIM_DEBUG
    if (!check_validation_layers()){
	    throw std::runtime_error("Requested validation layers not available.");
    }
#endif

    // determine vulkan version
    typedef VkResult (*vulkan_version_func_t)(uint32_t *);
    auto vk_version_func = (vulkan_version_func_t) vkGetInstanceProcAddr(instance, "vkEnumerateInstanceVersion");
    
    uint32_t version;
    int major, minor, patch;
    if (vk_version_func != nullptr && vk_version_func(&version) == VK_SUCCESS){
        major = version >> 22;
        minor = (version - (major << 22)) >> 12;
        patch = version - (major << 22) - (minor << 12);
    } else {
        major = 1;
        minor = 0;
        patch = 0;
    }

    std::cout << "Vulkan version: " << major << '.' << minor << '.' << patch << std::endl;

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
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
    for (uint32_t i = 0; i < create_info.enabledExtensionCount; i++){
        std::cout << "\t" << create_info.ppEnabledExtensionNames[i] << std::endl;
    }

#if SERAPHIM_DEBUG
    create_info.ppEnabledLayerNames = validation_layers.data();
    create_info.enabledLayerCount   = validation_layers.size();
#else
    create_info.enabledLayerCount   = 0;
#endif

    std::cout << "Enabled validation layers: "  << std::endl;
    for (uint32_t i = 0; i < create_info.enabledLayerCount; i++){
        std::cout << "\t" << create_info.ppEnabledLayerNames[i] << std::endl;
    }

    auto result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS){
	    throw std::runtime_error("Failed to create Vulkan instance!");
    }
}

#if SERAPHIM_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL 
debug_callback(
    VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT obj_type,
    uint64_t obj, size_t location, int32_t code,
    const char * layer_prefix, const char * msg, void * user_data
){
    std::cout << "Validation layer debug message: " << msg << std::endl;
    return VK_FALSE;
}

bool
seraphim_t::setup_debug_callback(){
    VkDebugReportCallbackCreateInfoEXT create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT 
                      | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    create_info.pfnCallback = debug_callback;

    //load in function address, since its an extension
    auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(
        instance, "vkCreateDebugReportCallbackEXT"
    );

    return func != nullptr && func(instance, &create_info, nullptr, &callback) == VK_SUCCESS;
}

bool
seraphim_t::check_validation_layers(){
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    return std::all_of(validation_layers.begin(), validation_layers.end(), [available_layers](auto & layer){
        return std::any_of(available_layers.begin(), available_layers.end(), [layer](auto & properties){
            return layer == std::string(properties.layerName);
        });
    });
}
#endif

void
seraphim_t::run(){
    uint32_t current_frame = 0;
    uint32_t frequency = 100;
    auto   previous   = std::chrono::steady_clock::now();

    while (!window->should_close()){
	    glfwPollEvents();

        auto now   = std::chrono::steady_clock::now();
        double delta = std::chrono::duration_cast<std::chrono::microseconds>(now - previous).count() / 1000000.0;
        previous = now;

        window->get_mouse().update(delta, *window);
        test_camera->update(delta, window->get_keyboard(), window->get_mouse());

        if (current_frame % frequency == 0){    
            std::cout << "FPS: " << 1.0 / delta << std::endl;
        }

        renderer->render();

        current_frame++;
    }
}

std::weak_ptr<window_t> 
seraphim_t::get_window() const {
    return window;
}
