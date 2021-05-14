#include "core/device.h"

#include <set>
#include <stdexcept>
#include <string>
#include <vector>

const std::vector<const char *> device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

static VkPhysicalDevice select_physical_device(VkInstance instance,
                                               VkSurfaceKHR surface);
static void select_queue_families(device_t * device, VkSurfaceKHR surface);

static bool device_has_extension(VkPhysicalDevice phys_device,
                                 const char *extension) {
    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(phys_device, nullptr, &extension_count,
                                         nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(phys_device, nullptr, &extension_count,
                                         available_extensions.data());

    for (auto available_extension : available_extensions) {
        if (std::string(extension) ==
            std::string(available_extension.extensionName)) {
            return true;
        }
    }

    return false;
}

static bool has_adequate_queue_families(VkPhysicalDevice physical_device,
                                        VkSurfaceKHR surface) {
    bool queue_families_found[3] = {false, false, false};

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count,
                                             queue_families.data());

    for (uint32_t i = 0; i < queue_family_count; i++) {
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface,
                                             &present_support);

        if (queue_families[i].queueCount > 0) {
            queue_families_found[0] |= present_support;
            queue_families_found[1] |=
                queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
            queue_families_found[2] |=
                queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
        }
    }

    return queue_families_found[0] && queue_families_found[1] &&
           queue_families_found[2];
}

static bool is_suitable_device(VkPhysicalDevice physical_device_,
                                  VkSurfaceKHR surface) {
    // check that gpu isnt integrated
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device_, &properties);
    if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        return false;
    }

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physical_device_, &features);
    // can do extra checks here if you want

    // check device has at least one graphics queue family
    if (!has_adequate_queue_families(physical_device_, surface)) {
        return false;
    }

    for (auto extension : device_extensions) {
        if (!device_has_extension(physical_device_, extension)) {
            return false;
        }
    }

    uint32_t formats_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface, &formats_count,
                                         nullptr);

    uint32_t present_modes_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface,
                                              &present_modes_count, nullptr);
    if (formats_count * present_modes_count == 0) {
        return false;
    }

    return true;
}

static VkPhysicalDevice select_physical_device(VkInstance instance,
                                                  VkSurfaceKHR surface) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    for (auto physical_device_ : devices) {
        if (is_suitable_device(physical_device_, surface)) {
            return physical_device_;
        }
    }

    throw std::runtime_error("Error: Unable to find a suitable physical device!");
}

static void select_queue_families(device_t * device, VkSurfaceKHR surface) {
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device->physical_device, &queue_family_count,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device->physical_device, &queue_family_count,
                                             queue_families.data());

    for (uint32_t i = 0; i < queue_family_count; i++) {
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device->physical_device, i, surface,
                                             &present_support);

        if (queue_families[i].queueCount > 0) {
            if (present_support) {
                device->present_family = i;
            }

            if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                device->graphics_family = i;
            }

            if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                device->compute_family = i;
            }
        }
    }
}

uint32_t device_memory_type(device_t *device, uint32_t type_filter,
                            VkMemoryPropertyFlags prop) {
    VkPhysicalDeviceMemoryProperties mem_prop;
    vkGetPhysicalDeviceMemoryProperties(device->physical_device, &mem_prop);

    for (uint32_t i = 0; i < mem_prop.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) &&
            (mem_prop.memoryTypes[i].propertyFlags & prop) == prop) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void device_create(device_t *device, VkInstance instance, VkSurfaceKHR surface,
                   std::vector<const char *> enabled_validation_layers) {
    device->physical_device = select_physical_device(instance, surface);
    select_queue_families(device, surface);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {device->graphics_family, device->present_family,
                                                device->compute_family};

    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    for (uint32_t queue_family : unique_queue_families) {
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.queueCreateInfoCount =
            static_cast<uint32_t>(queue_create_infos.size());
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = device_extensions.size();
    create_info.ppEnabledExtensionNames = device_extensions.data();
    create_info.enabledLayerCount =
            static_cast<uint32_t>(enabled_validation_layers.size());
    create_info.ppEnabledLayerNames = enabled_validation_layers.data();

    if (vkCreateDevice(device->physical_device, &create_info, nullptr, &device->device) !=
        VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create device");
    }
}

void device_destroy(device_t *device) {
    vkDestroyDevice(device->device, nullptr);
}
