#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "core/allocator.h"
#include "maths/vec.h"

class swapchain_t {
public:
    swapchain_t(
        const allocator_t & allocator, u32vec2_t size,
        VkSurfaceKHR surface, uint32_t graphics_family,
        uint32_t present_family
    );
    ~swapchain_t();

    VkFormat get_image_format() const;
    VkExtent2D get_extents() const;
    VkSwapchainKHR get_handle() const;
    uint32_t get_size() const;
    VkImageView get_image_view(uint32_t i) const;

private:
    VkSurfaceFormatKHR select_surface_format(VkSurfaceKHR surface); 
    VkPresentModeKHR select_present_mode(VkSurfaceKHR surface);
    VkExtent2D select_swap_extent(u32vec2_t size, VkSurfaceKHR surface);
    allocator_t allocator;

    VkSwapchainKHR handle;
    std::vector<VkImageView> image_views;
    VkFormat image_format;
    VkExtent2D extents;
};

#endif