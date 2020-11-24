#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <memory>
#include <vector>

#include "core/device.h"
#include "maths/matrix.h"

namespace srph {
    class swapchain_t {
    public:
        swapchain_t(device_t * device, u32vec2_t size, VkSurfaceKHR surface);
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
        
        device_t * device;
        VkSwapchainKHR handle;
        std::vector<VkImageView> image_views;
        VkFormat image_format;
        VkExtent2D extents;
    };
}

#endif
