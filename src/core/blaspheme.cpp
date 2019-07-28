#include "core/blaspheme.h"

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

const std::vector<const char *> validation_layers = {
    "VK_LAYER_LUNARG_standard_validation"
};

blaspheme_t::blaspheme_t(){
#if BLASPHEME_DEBUG
    std::cout << "Running in debug mode." << std::endl;
#else 
    std::cout << "Running in release mode." << std::endl;
#endif

    // initialise GLFW
    glfwInit();

    scheduler = std::make_shared<scheduler_t>();
    window = std::make_shared<window_t>(u32vec2_t(640u, 480u), scheduler);

    // initialise vulkan
    create_instance();

    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

    std::cout << "Available extensions:" << std::endl;
    for (const auto & extension : extensions) {
	    std::cout << "\t" << extension.extensionName << std::endl;
    }

    if (is_debug){
	    if (!setup_debug_callback()){
	        throw std::runtime_error("Error: Failed to setup debug callback.");
	    }
    }

    if (glfwCreateWindowSurface(instance, window->get_window(), nullptr, &surface) != VK_SUCCESS) {
	    throw std::runtime_error("Error: Failed to create window surface.");
    }

    device = std::make_shared<device_t>(instance, surface);


    // VkPhysicalDeviceProperties properties = {};
    // vkGetPhysicalDeviceProperties(allocator.physical_device, &properties);
    // std::cout << "Chosen physical device: " << properties.deviceName << std::endl;

    // std::cout << "\tMaximum 1D image dimension: " << properties.limits.maxImageDimension1D << std::endl;
    // std::cout << "\tMaximum 2D image dimension: " << properties.limits.maxImageDimension2D << std::endl;
    // std::cout << "\tMaximum 3D image dimension: " << properties.limits.maxImageDimension3D << std::endl;
    // std::cout << "\tMaximum storage buffer range: " << properties.limits.maxStorageBufferRange << std::endl;

    // auto work_group_size = properties.limits.maxComputeWorkGroupSize;
    // auto work_group_count = properties.limits.maxComputeWorkGroupCount;
    // std::cout << "\tMaximum work group count: " << work_group_count[0] << ", " << work_group_count[1] << ", " << work_group_count[2] << std::endl;
    // std::cout << "\tMaximum work group size: " << work_group_size[0] << ", " << work_group_size[1] << ", " << work_group_size[2] << std::endl;


    // uint32_t push_const_size = properties.limits.maxPushConstantsSize;
    // std::cout << "\tMaximum push constants size: " << push_const_size << ". Push constants data structure size: " << sizeof(renderer_t::push_constant_t) << std::endl;
    // if (sizeof(renderer_t::push_constant_t) > push_const_size){
    //     // TODO: put this check when selecting physical device
    //     throw std::runtime_error("Error: Push constants too large.");
    // }


    // initialise vulkan memory allocator
    VmaAllocatorCreateInfo allocator_info = {};
    allocator_info.physicalDevice = device->get_physical_device();
    allocator_info.device = device->get_device();
    
    vmaCreateAllocator(&allocator_info, &allocator);


    // frame_start_follower = scheduler->on_frame_start.follow(std::bind(
    //     &blaspheme_t::update_fps_counter, this, std::placeholders::_1
    // ));

    test_camera = std::make_shared<camera_t>(this);

    renderer = std::make_shared<renderer_t>(
        allocator, device, surface, window, test_camera
    );
}

blaspheme_t::~blaspheme_t(){
    vkDeviceWaitIdle(device->get_device());

    // delete renderer early to release resources at appropriate time
    renderer.reset();

    vmaDestroyAllocator(allocator);

    // destroy device
    device.reset();

    // destroy debug callback
    if (is_debug){
        auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(
            instance, "vkDestroyDebugReportCallbackEXT"
        );

        if (func != nullptr){
            func(instance, callback, nullptr);
        }
    }
   
    vkDestroySurfaceKHR(instance, surface, nullptr);

    // destroy instance
    vkDestroyInstance(instance, nullptr);

    window.reset();

    glfwTerminate();
}

std::weak_ptr<renderer_t> 
blaspheme_t::get_renderer() const {
    return renderer;
}


std::weak_ptr<scheduler_t> 
blaspheme_t::get_scheduler() const {
    return scheduler;
}

bool
blaspheme_t::check_validation_layers(){
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (auto layer : validation_layers){
        bool found = false;
	
        for (auto layer_property : available_layers){ 
            if (layer == std::string(layer_property.layerName)){
                found = true;
                break;
            }
        }

        if (!found){
            return false;
        }
    }

    return true;
}

std::vector<const char *>
blaspheme_t::get_required_extensions(){
    uint32_t      extension_count = 0;
    const char ** glfw_extensions = glfwGetRequiredInstanceExtensions(&extension_count);

    std::vector<const char *> req_ext(glfw_extensions, glfw_extensions + extension_count);
    if (is_debug){
	    req_ext.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    return req_ext;
}



void
blaspheme_t::create_instance(){
    if (is_debug && !check_validation_layers()){
	    throw std::runtime_error("Requested validation layers not available.");
    }

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
    app_info.pApplicationName = "Chalet test";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "BLASPHEME";
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

    if (is_debug){
        create_info.ppEnabledLayerNames = validation_layers.data();
        create_info.enabledLayerCount   = validation_layers.size();
    } else {
        create_info.enabledLayerCount   = 0;
    }

    std::cout << "Enabled validation layers: "  << std::endl;
    for (uint32_t i = 0; i < create_info.enabledLayerCount; i++){
        std::cout << "\t" << create_info.ppEnabledLayerNames[i] << std::endl;
    }

    auto result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS){
	    throw std::runtime_error("Failed to create Vulkan instance!");
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL 
debug_callback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT obj_type,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char * layer_prefix,
    const char * msg,
    void * user_data
){
    std::cout << "Validation layer debug message: " << msg << std::endl;
    return VK_FALSE;
}

bool
blaspheme_t::setup_debug_callback(){
    if (!is_debug){
	    return true;
    }

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

void
blaspheme_t::run(){
    while (!window->should_close()){
	    glfwPollEvents();
        scheduler->on_frame_start.tick();
        renderer->render();
    }
}

void
blaspheme_t::update_fps_counter(double delta){
    std::string title = "BLASPHEME | FPS: " + std::to_string(1.0 / delta);
    window->set_title(title);
}

std::weak_ptr<window_t> 
blaspheme_t::get_window() const {
    return window;
}