#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <memory>
#include <vector>

#include "device.h"
#include "../common/camera.h"

typedef struct swapchain_t {
    swapchain_t(device_t *device, vec2u *size, VkSurfaceKHR surface);
    ~swapchain_t();

    uint32_t get_size() const;

    device_t *device;
    VkSwapchainKHR handle;
    std::vector<VkImageView> image_views;
    VkFormat image_format;
    VkExtent2D extents;
} swapchain_t;

#endif
