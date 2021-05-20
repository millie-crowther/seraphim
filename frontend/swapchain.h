#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <memory>
#include <vector>

#include "device.h"
#include "../common/camera.h"

struct swapchain_t {
    swapchain_t(device_t *device, vec2u *size, VkSurfaceKHR surface);
    ~swapchain_t();

    uint32_t get_size() const;
    VkImageView get_image_view(uint32_t i) const;

    VkSurfaceFormatKHR select_surface_format(VkSurfaceKHR surface);
    VkPresentModeKHR select_present_mode(VkSurfaceKHR surface);
    VkExtent2D select_swap_extent(vec2u *size, VkSurfaceKHR surface);

    device_t *device;
    VkSwapchainKHR handle;
    std::vector<VkImageView> image_views;
    VkFormat image_format;
    VkExtent2D extents;
};

#endif
