#ifndef DEVICE_H
#define DEVICE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

typedef struct device_t {
    VkPhysicalDevice physical_device;
    VkDevice device;

    uint32_t graphics_family;
    uint32_t present_family;
    uint32_t compute_family;

    VkPhysicalDevice select_physical_device(VkInstance instance,
                                            VkSurfaceKHR surface) const;
    bool is_suitable_device(VkPhysicalDevice physical_device,
                            VkSurfaceKHR surface) const;
    void select_queue_families(VkSurfaceKHR surface);

    device_t(VkInstance instance, VkSurfaceKHR surface,
             std::vector<const char *> enabled_validation_layers);
    ~device_t();
} device_t;

uint32_t device_memory_type(device_t *device, uint32_t type_filter,
                            VkMemoryPropertyFlags prop);

#endif
