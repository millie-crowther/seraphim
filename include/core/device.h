#ifndef DEVICE_H
#define DEVICE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

class device_t {
private:
    VkPhysicalDevice physical_device;
    VkDevice device;

    uint32_t graphics_family;
    uint32_t present_family;
    uint32_t compute_family;

    VkPhysicalDevice select_physical_device(VkInstance instance, VkSurfaceKHR surface) const;
    bool has_adequate_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface) const;
    bool is_suitable_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface) const;

    VkDevice create_device(std::vector<const char *> enabled_validation_layers) const;
    bool device_has_extension(VkPhysicalDevice phys_device, const char * extension) const;
    void select_queue_families(VkSurfaceKHR surface);

public:
    device_t(VkInstance instance, VkSurfaceKHR surface, std::vector<const char *> enabled_validation_layers);
    ~device_t();

    VkPhysicalDevice get_physical_device() const;
    VkDevice get_device() const;
    uint32_t get_graphics_family() const;
    uint32_t get_present_family() const;
    uint32_t get_compute_family() const;
};

#endif
