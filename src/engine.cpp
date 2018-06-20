#include "engine.h"

#include <iostream>
#include <stdexcept>
#include <string>

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

    setup_debug_callback();
}

bool
engine_t::check_validation_layers(std::vector<const char*> validation_layers){
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

void
engine_t::create_instance(){
    const std::vector<const char *> validation_layers = {
        "VK_LAYER_LUNARG_standard_validation"
    };


    if (is_debug && !check_validation_layers(validation_layers)){
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
    if (is_debug){
        auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr){
	    func(instance, callback, nullptr);
	}
    }
    
    vkDestroyInstance(instance, nullptr);

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
