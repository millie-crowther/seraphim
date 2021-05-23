#include "swapchain.h"

#include "texture.h"

#include <algorithm>
#include "../common/debug.h"

static VkSurfaceFormatKHR select_surface_format(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
static VkExtent2D select_swap_extent(VkPhysicalDevice physical_device, vec2u *size, VkSurfaceKHR surface);
static VkPresentModeKHR select_present_mode(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

swapchain_t::swapchain_t(device_t *device, vec2u *size, VkSurfaceKHR surface) {
    this->device = device;
    VkSurfaceFormatKHR format = select_surface_format(device->physical_device, surface);
    VkPresentModeKHR mode = select_present_mode(device->physical_device, surface);
    extents = select_swap_extent(device->physical_device, size, surface);

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physical_device, surface,
                                              &capabilities);
    uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount != 0 &&
        image_count > capabilities.maxImageCount) {
        image_count = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = format.format;
    create_info.imageColorSpace = format.colorSpace;
    create_info.imageExtent = extents;
    create_info.imageArrayLayers = 1;
    // if you dont wanna draw to image directly VK_IMAGE_USAGE_TRANSFER_DST_BIT
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t families[2] = {device->graphics_family, device->present_family};

    if (families[0] != families[1]) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = families;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = NULL;
    }

    create_info.preTransform = capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device->device, &create_info, NULL, &handle) !=
        VK_SUCCESS) {
        PANIC("Error: failed to create swapchain!");
    }

    uint32_t count = 0;
    vkGetSwapchainImagesKHR(device->device, handle, &count, NULL);

    std::vector<VkImage> images(count);
    vkGetSwapchainImagesKHR(device->device, handle, &count, images.data());

    image_views.clear();
    image_format = format.format;
    for (auto &image : images) {
        image_views.push_back(
                texture_create_image_view(device->device, image, format.format));
    }
}

swapchain_t::~swapchain_t() {
    for (auto image_view : image_views) {
        vkDestroyImageView(device->device, image_view, NULL);
    }
    image_views.clear();

    vkDestroySwapchainKHR(device->device, handle, NULL);
}

static VkSurfaceFormatKHR select_surface_format(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count,
                                         NULL);
    VkSurfaceFormatKHR formats[count];
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count, formats);

    // check if all formats supported
    if (count == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    // check for preferred
    for (uint32_t i = 0; i < count; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
            formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return formats[i];
        }
    }

    // default
    return formats[0];
}

static VkExtent2D select_swap_extent(VkPhysicalDevice physical_device, vec2u *size, VkSurfaceKHR surface) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface,
                                              &capabilities);

    // check if we need to supply width and height
    if (capabilities.currentExtent.width == ~((uint32_t)0)) {
        VkExtent2D extents_;

        extents_.width =
            std::max(capabilities.minImageExtent.width,
                     std::min(size->x, capabilities.maxImageExtent.width));
        extents_.height =
            std::max(capabilities.minImageExtent.height,
                     std::min(size->y, capabilities.maxImageExtent.height));

        return extents_;
    } else {
        return capabilities.currentExtent;
    }
}

static VkPresentModeKHR select_present_mode(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                              &count, NULL);
    VkPresentModeKHR modes[count];
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface,
                                              &count, modes);

    for (uint32_t i = 0; i < count; i++){
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR){
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }

    for (uint32_t i = 0; i < count; i++){
        if (modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR){
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

uint32_t swapchain_t::get_size() const { return image_views.size(); }

