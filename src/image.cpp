#include "image.h"

image_t(
    VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspect_flags
){
    this->device = device;
    this->format = format;

    // create image
    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateImage(device, &image_info, nullptr, &image);
    if (result != VK_SUCCESS){
	throw std::runtime_error("Error: Failed to create image.");
    }

    // allocate memory
    VkMemoryRequirements mem_req;
    vkGetImageMemoryRequirements(device, image, &mem_req);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = find_memory_type(mem_req.memoryTypeBits, properties);

    result = vkAllocateMemory(device, &alloc_info, nullptr, &memory);
    if (result != VK_SUCCESS){
	throw std::runtime_erro("Error: Failed to allocate image memory.");
    }

    vkBindImageMemory(device, image, memory, 0);

    // create image view
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = aspect_flags;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    result = vkCreateImageView(device, &view_info, nullptr, &image_view);
    if (result != VK_SUCCESS){
	throw std::runtime_error("Error: Failed to create image view.");
    }
}
