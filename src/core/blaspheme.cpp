#include "core/blaspheme.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <set>
#include <algorithm>
#include <fstream>
#include <cstring>
#include "maths/maths.h"
#include <memory>
#include "render/renderer.h"

#include "logic/scheduler.h"

VkPhysicalDevice blaspheme_t::physical_device;
VkDevice blaspheme_t::device;

const std::vector<const char *> validation_layers = {
    "VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char *> device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static void 
window_resize_callback(GLFWwindow * window, int width, int height){
    void * data = glfwGetWindowUserPointer(window);
    blaspheme_t * blaspheme = reinterpret_cast<blaspheme_t *>(data);
    blaspheme->window_resize(u32vec2_t((uint32_t) width, (uint32_t) height));
}

static void 
key_callback(GLFWwindow * window, int key, int scancode, int action, int mods){
    void * data = glfwGetWindowUserPointer(window);
    blaspheme_t * blaspheme = reinterpret_cast<blaspheme_t *>(data);
    blaspheme->keyboard_event(key, action, mods);
}

blaspheme_t::blaspheme_t(bool is_debug){
    this->is_debug = is_debug;

    std::cout << "Running in " << (is_debug ? "debug" : "release") << " mode." << std::endl;
    // initialise GLFW
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    u32vec2_t window_size(640u, 480u);

    window = glfwCreateWindow(window_size[0], window_size[1], "BLASPHEME", nullptr, nullptr);

    glfwSetWindowUserPointer(window, static_cast<void *>(this));

    glfwSetWindowSizeCallback(window, window_resize_callback);   
    glfwSetKeyCallback(window, key_callback);

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

    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
	    throw std::runtime_error("Error: Failed to create window surface.");
    }

    physical_device = select_device();
    if (physical_device == VK_NULL_HANDLE){
	    throw std::runtime_error("Error: Couldn't find an appropriate GPU.");
    }

    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    std::cout << "Chosen physical device: " << properties.deviceName << std::endl;

    std::cout << "\tMaximum 2D image dimension: " << properties.limits.maxImageDimension2D << std::endl;
    
    uint32_t push_const_size = properties.limits.maxPushConstantsSize;
    std::cout << "\tMaximum push constants size: " << push_const_size << std::endl;
    if (sizeof(renderer_t::push_constant_t) > push_const_size){
        // TODO: put this check when selecting physical device
        throw std::runtime_error("Error: Push constants too large.");
    }
    
    if (!create_logical_device()){
	    throw std::runtime_error("Error: Couldn't create logical device.");
    }

    // initialise vulkan memory allocator
    VmaAllocatorCreateInfo allocator_info = {};
    allocator_info.physicalDevice = physical_device;
    allocator_info.device = device;
    
    vmaCreateAllocator(&allocator_info, &allocator);

    uint32_t graphics_family = get_graphics_queue_family(physical_device);
    uint32_t present_family  = get_present_queue_family(physical_device);

    renderer = std::make_unique<renderer_t>(
        allocator, physical_device, device, surface, graphics_family, present_family, window_size, &keyboard
    );
}

blaspheme_t::~blaspheme_t(){
    // scheduler::halt();    

    vkDeviceWaitIdle(device);

    // delete renderer early to release resources at appropriate time
    renderer.reset(nullptr);

    vmaDestroyAllocator(allocator);

    // destroy logical device
    vkDestroyDevice(device, nullptr);

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

    // destory GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

}

void
blaspheme_t::window_resize(const u32vec2_t & size){
    renderer->window_resize(size);
}

void 
blaspheme_t::keyboard_event(int key, int action, int mods){
    keyboard.key_event(key, action, mods);
}

bool
blaspheme_t::create_logical_device(){
    uint32_t graphics = get_graphics_queue_family(physical_device);
    uint32_t present = get_present_queue_family(physical_device);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = { graphics, present };
    
    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    for (uint32_t queue_family : unique_queue_families){
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy        = VK_TRUE;

    VkDeviceCreateInfo create_info      = {};
    create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos       = queue_create_infos.data();
    create_info.queueCreateInfoCount    = static_cast<uint32_t>(queue_create_infos.size());
    create_info.pEnabledFeatures        = &device_features;
    create_info.enabledExtensionCount   = device_extensions.size();
    create_info.ppEnabledExtensionNames = device_extensions.data();

    if (is_debug) {
        create_info.enabledLayerCount   = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
    } else {
	    create_info.enabledLayerCount   = 0;
    } 

    if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS){
	    return false;
    }

    return true;
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

int
blaspheme_t::get_graphics_queue_family(VkPhysicalDevice phys_device){
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> q_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, q_families.data());
   
    for (int i = 0; i < queue_family_count; i++){
        if (q_families[i].queueCount > 0 && q_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            return i;
        }
    }

    return -1;    
}

int 
blaspheme_t::get_present_queue_family(VkPhysicalDevice phys_device){
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> q_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, q_families.data());
   
    for (int i = 0; i < queue_family_count; i++){
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(phys_device, i, surface, &present_support);

        if (q_families[i].queueCount > 0 && present_support){
            return i;
        }
    }

    return -1;    
}

bool
blaspheme_t::has_adequate_swapchain(VkPhysicalDevice physical_device){
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count, nullptr);
    if (count == 0){
	    return false;
    }

    count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &count, nullptr);
    if (count == 0){
        return false;
    }

    return true;
}

bool
blaspheme_t::is_suitable_device(VkPhysicalDevice phys_device){
    // check that gpu isnt integrated
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(phys_device, &properties);
    if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
        return false;
    }

    // check device can do geometry shaders
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(phys_device, &features);
    if (!features.geometryShader || ! features.samplerAnisotropy){
	    return false;
    }

    // check device has at least one graphics queue family
    if (get_graphics_queue_family(phys_device) == -1){
	    return false;
    }

    if (get_present_queue_family(phys_device) == -1){
	    return false;
    }

    for (auto extension : device_extensions){
        if (!device_has_extension(phys_device, extension)){
            return false;
        }
    }

    if (!has_adequate_swapchain(phys_device)){
	    return false;
    }

    return true;
}

bool
blaspheme_t::device_has_extension(VkPhysicalDevice phys_device, const char * extension){
    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(phys_device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(
        phys_device, nullptr, &extension_count, available_extensions.data()
    );

    for (auto available_extension : available_extensions){
        if (std::string(extension) == std::string(available_extension.extensionName)){
            return true;
        }
    }

    return false;
}

VkPhysicalDevice
blaspheme_t::select_device(){
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    if (device_count == 0){
 	    return VK_NULL_HANDLE;
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    
    for (auto phys_device : devices){
        if (is_suitable_device(phys_device)){
            return phys_device;
        }
    }

    return VK_NULL_HANDLE;
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
    create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    create_info.ppEnabledExtensionNames = required_extensions.data();

    std::cout << "Enabled extensions: " << std::endl;
    for (int i = 0; i < create_info.enabledExtensionCount; i++){
        std::cout << "\t" << create_info.ppEnabledExtensionNames[i] << std::endl;
    }

    if (is_debug){
        create_info.ppEnabledLayerNames = validation_layers.data();
        create_info.enabledLayerCount   = validation_layers.size();
    } else {
        create_info.enabledLayerCount   = 0;
    }

    std::cout << "Enabled validation layers: "  << std::endl;
    for (int i = 0; i < create_info.enabledLayerCount; i++){
        std::cout << "\t" << create_info.ppEnabledLayerNames[i] << std::endl;
    }

    auto result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS){
        std::cout << result << " : " << VK_ERROR_INCOMPATIBLE_DRIVER << std::endl;
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

bool
blaspheme_t::should_quit(){
    return glfwWindowShouldClose(window) || keyboard.is_key_pressed(GLFW_KEY_ESCAPE);
}

void
blaspheme_t::run(){
    while (!should_quit()){
	    glfwPollEvents();
        renderer->render();
    }
}

VkPhysicalDevice
blaspheme_t::get_physical_device(){
    return physical_device;
}

VkDevice
blaspheme_t::get_device(){
    return device;
}