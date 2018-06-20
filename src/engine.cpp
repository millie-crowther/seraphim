#include "engine.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <set>
#include <algorithm>

const std::vector<const char *> validation_layers = {
    "VK_LAYER_LUNARG_standard_validation"
};

engine_t::engine_t(bool is_debug){
    this->is_debug = is_debug;

    if (is_debug){
	std::cout << "Running in debug mode." << std::endl;
    } else {
	std::cout << "Running in release mode." << std::endl;
    }
}

void
engine_t::init(){
    // initialise GLFW
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    int WIDTH = 640;
    int HEIGHT = 480;
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    // initialise vulkan
    create_instance();

    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

    std::cout << "Available extensions:" << std::endl;
    for (const auto& extension : extensions) {
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

    VkPhysicalDevice physical_device = select_device();
    if (physical_device == VK_NULL_HANDLE){
	throw std::runtime_error("Error: Couldn't find a Vulkan compatible GPU.");
    }

    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    std::cout << "Chosen physical device: " << properties.deviceName << std::endl;

    if (!create_logical_device(physical_device)){
	throw std::runtime_error("Error: Couldn't create logical device.");
    }

    int index = get_graphics_queue_family(physical_device);
    vkGetDeviceQueue(device, index, 0, &graphics_queue);
}

bool
engine_t::create_logical_device(VkPhysicalDevice physical_device){
    int graphics = get_graphics_queue_family(physical_device);
    int present = get_present_queue_family(physical_device);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<int> unique_queue_families = { graphics, present };
    
    float queue_priority = 1.0f;
    for (int queue_family : unique_queue_families){
	VkDeviceQueueCreateInfo queue_create_info = {};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueFamilyIndex = queue_family;
	queue_create_info.queueCount = 1;
	queue_create_info.pQueuePriorities = &queue_priority;
	queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features = {};

    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());

    create_info.pEnabledFeatures = &device_features;

    create_info.enabledExtensionCount = 0;

    if (is_debug) {
	create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
	create_info.ppEnabledLayerNames = validation_layers.data();
    } else {
	create_info.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS){
	return false;
    }

    vkGetDeviceQueue(device, graphics, 0, &graphics_queue);
    vkGetDeviceQueue(device, present, 0, &present_queue);

    return true;
}

bool
engine_t::check_validation_layers(){
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
engine_t::get_required_extensions(){
    uint32_t extension_count = 0;
    const char ** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&extension_count);

    std::vector<const char *> req_ext(glfw_extensions, glfw_extensions + extension_count);
    if (is_debug){
	req_ext.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    return req_ext;
}

int
engine_t::get_graphics_queue_family(VkPhysicalDevice device){
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> q_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, q_families.data());
   
    for (int i = 0; i < queue_family_count; i++){
	if (q_families[i].queueCount > 0 && q_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
	    return i;
	}
    }

    return -1;    
}

int 
engine_t::get_present_queue_family(VkPhysicalDevice physical_device){
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> q_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, q_families.data());
   
    for (int i = 0; i < queue_family_count; i++){
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);

	if (q_families[i].queueCount > 0 && present_support){
	    return i;
	}
    }

    return -1;    
}

bool
engine_t::is_suitable_device(VkPhysicalDevice physical_device){
    // check that gpu isnt integrated
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
        return false;
    }

    // check device can do geometry shaders
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physical_device, &features);
    if (!features.geometryShader){
	return false;
    }

    // check device has at least one graphics queue family
    if (get_graphics_queue_family(physical_device) == -1){
	return false;
    }

    if (get_present_queue_family(physical_device) == -1){
	return false;
    }

    return true;
}

VkPhysicalDevice
engine_t::select_device(){
    uint32_t device_count = 0;
    
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    if (device_count == 0){
	return VK_NULL_HANDLE;
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    
    for (auto physical_device : devices){
	if (is_suitable_device(physical_device)){
            return physical_device;
	}
    }

    return VK_NULL_HANDLE;
}

void
engine_t::create_instance(){
    if (is_debug && !check_validation_layers()){
	throw std::runtime_error("Requested validation layers not available.");
    }

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Hello Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    auto required_extensions = get_required_extensions(); 

    create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    create_info.ppEnabledExtensionNames = required_extensions.data();

    if (is_debug){
	create_info.ppEnabledLayerNames = validation_layers.data();
	create_info.enabledLayerCount = validation_layers.size();
    } else {
        create_info.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS){
	throw std::runtime_error("Failed to create Vulkan instance!");
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL 
debug_callback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT obj_type,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char* layer_prefix,
    const char* msg,
    void* user_data
){
    std::cerr << "Validation layer debug message: " << msg << std::endl;
    return VK_FALSE;
}

bool
engine_t::setup_debug_callback(){
    if (!is_debug){
	return true;
    }

    VkDebugReportCallbackCreateInfoEXT create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    create_info.pfnCallback = debug_callback;

    //load in function address, since its an extension
    auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if (func == nullptr){
        return false;
    }

    if (func(instance, &create_info, nullptr, &callback) != VK_SUCCESS){
	return false;
    }	

    return true;
}

void 
engine_t::update(){
    glfwPollEvents();
}

void
engine_t::cleanup(){
    // destory logical device
    vkDestroyDevice(device, nullptr);

    // destroy debug callback
    if (is_debug){
        auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
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

bool
engine_t::should_quit(){
    return glfwWindowShouldClose(window);
}

void
engine_t::run(){
    init();	

    while (!should_quit()){
	update();
    }

    cleanup();
}
