#include "render/swapchain.h"

#include "render/texture.h"

using namespace srph;

swapchain_t::swapchain_t(
    device_t * device, u32vec2_t size,
    VkSurfaceKHR surface
){
    this->device = device;
    VkSurfaceFormatKHR format = select_surface_format(surface);
    VkPresentModeKHR mode = select_present_mode(surface);
    extents = select_swap_extent(size, surface);

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device->get_physical_device(), surface, &capabilities
    );
    uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount != 0 && image_count > capabilities.maxImageCount){
	    image_count = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface          = surface;
    create_info.minImageCount    = image_count;
    create_info.imageFormat      = format.format;
    create_info.imageColorSpace  = format.colorSpace;
    create_info.imageExtent      = extents;
    create_info.imageArrayLayers = 1;
    // if you dont wanna draw to image directly VK_IMAGE_USAGE_TRANSFER_DST_BIT
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;     

    uint32_t families[2] = { device->get_graphics_family(), device->get_present_family() };

    if (families[0] != families[1]){
        create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
	    create_info.pQueueFamilyIndices   = families;
    } else {
	    create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
	    create_info.pQueueFamilyIndices   = nullptr;
    }

    create_info.preTransform   = capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode    = mode;
    create_info.clipped        = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device->get_device(), &create_info, nullptr, &handle) != VK_SUCCESS){
	    throw std::runtime_error("Error: failed to create swapchain!");
    }

    uint32_t count = 0;
    vkGetSwapchainImagesKHR(device->get_device(), handle, &count, nullptr);

    std::vector<VkImage> images(count);
    vkGetSwapchainImagesKHR(device->get_device(), handle, &count, images.data());
  
    image_views.clear();
    image_format = format.format;
    for (auto & image : images){
        image_views.push_back(
            texture_t::create_image_view(device->get_device(), image, format.format)
        );
    }
}

swapchain_t::~swapchain_t(){
    for (auto image_view : image_views){
        vkDestroyImageView(device->get_device(), image_view, nullptr);
    }
    image_views.clear();

    vkDestroySwapchainKHR(device->get_device(), handle, nullptr);
}

VkSurfaceFormatKHR swapchain_t::select_surface_format(VkSurfaceKHR surface){
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        device->get_physical_device(), surface, &count, nullptr
    );
    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        device->get_physical_device(), surface, &count, formats.data()
    );
    
    // check if all formats supported
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED){
	    return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }

    // check for preferred
    for (auto available_format : formats){
        if (
            available_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        ){
            return available_format;
        }
    }

    // default
    return formats[0];
}

VkExtent2D swapchain_t::select_swap_extent(u32vec2_t size, VkSurfaceKHR surface){
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device->get_physical_device(), surface, &capabilities
    );

    // check if we need to supply width and height
    if (capabilities.currentExtent.width == ~((uint32_t) 0)){
        VkExtent2D extents;
        
        extents.width = std::max(
            capabilities.minImageExtent.width, 
            std::min(size[0], capabilities.maxImageExtent.width)
        );
        extents.height = std::max(
            capabilities.minImageExtent.height, 
            std::min(size[1], capabilities.maxImageExtent.height)
        );
            
        return extents;
    } else {
        return capabilities.currentExtent;
    }
}

VkPresentModeKHR swapchain_t::select_present_mode(VkSurfaceKHR surface){
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device->get_physical_device(), surface, &count, nullptr
    );
    std::vector<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device->get_physical_device(), surface, &count, modes.data()
    );

    if (std::find(modes.begin(), modes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != modes.end()){
	    return VK_PRESENT_MODE_MAILBOX_KHR;
    }

    if (std::find(modes.begin(), modes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR) != modes.end()){
	    return VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}


VkFormat swapchain_t::get_image_format() const {
    return image_format;
}

VkExtent2D swapchain_t::get_extents() const {
    return extents;
}

VkSwapchainKHR swapchain_t::get_handle() const {
    return handle;
}

uint32_t swapchain_t::get_size() const {
    return image_views.size();
}

VkImageView swapchain_t::get_image_view(uint32_t i) const {
    return image_views[i];
}
