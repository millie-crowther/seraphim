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
} device_t;


void device_create(device_t * device, VkInstance instance, VkSurfaceKHR surface,
        std::vector<const char *> enabled_validation_layers);
void device_destroy(device_t * device);
uint32_t device_memory_type(device_t *device, uint32_t type_filter,
                            VkMemoryPropertyFlags prop);

#endif
