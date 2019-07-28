#include "core/device.h"

#include <vector>
#include <set>
#include <stdexcept>
#include <string>

const std::vector<const char *> device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

device_t::device_t(VkInstance instance, VkSurfaceKHR surface){
    physical_device = select_physical_device(instance, surface);
    select_queue_families(surface);
    device = create_device();
}

bool 
device_t::is_suitable_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface) const {
    // check that gpu isnt integrated
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
        return false;
    }

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physical_device, &features);
    // can do extra checks here if you want

    // check device has at least one graphics queue family
    if (!has_adequate_queue_families(physical_device, surface)){
        return false;
    }

    for (auto extension : device_extensions){
        if (!device_has_extension(physical_device, extension)){
            return false;
        }
    }

    uint32_t formats_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formats_count, nullptr);

    uint32_t present_modes_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_modes_count, nullptr);
    if (formats_count * present_modes_count == 0){
        return false;
    }

    return true;
}

bool
device_t::device_has_extension(VkPhysicalDevice phys_device, const char * extension) const {
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
device_t::select_physical_device(VkInstance instance, VkSurfaceKHR surface) const {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
   
    for (auto physical_device : devices){
        if (is_suitable_device(physical_device, surface)){
            return physical_device;
        }
    }

    throw std::runtime_error("Error: Unable to find a suitable physical device!");
}


bool 
device_t::has_adequate_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface) const {
    bool queue_families_found[3] = { false, false, false };

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());
   
    for (uint32_t i = 0; i < queue_family_count; i++){
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);

        if (queue_families[i].queueCount > 0){
            queue_families_found[0] |= present_support;
            queue_families_found[1] |= queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
            queue_families_found[2] |= queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
        }
    }

    return queue_families_found[0] && queue_families_found[1] && queue_families_found[2];
}

void 
device_t::select_queue_families(VkSurfaceKHR surface){
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());
   
    for (uint32_t i = 0; i < queue_family_count; i++){
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);

        if (queue_families[i].queueCount > 0){
            if (present_support){
                present_family = i;
            }

            if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
                graphics_family = i;
            }

            if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT){
                compute_family = i;
            }
        }
    }
}

VkDevice 
device_t::create_device() const {
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = { graphics_family, present_family, compute_family };
    
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

#if BLASPHEME_DEBUG
    create_info.enabledLayerCount   = static_cast<uint32_t>(validation_layers.size());
    create_info.ppEnabledLayerNames = validation_layers.data();
#else
    create_info.enabledLayerCount   = 0;
#endif 

    VkDevice device;
    if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS){
	    throw std::runtime_error("Error: failed to create device");
    }    

    return device;
}

device_t::~device_t(){
    vkDestroyDevice(device, nullptr);
}

VkPhysicalDevice
device_t::get_physical_device() const {
    return physical_device;
}

VkDevice 
device_t::get_device() const {
    return device;
}

uint32_t
device_t::get_compute_family() const {
    return compute_family;
}

uint32_t
device_t::get_graphics_family() const {
    return graphics_family;
}

uint32_t
device_t::get_present_family() const {
    return present_family;
}