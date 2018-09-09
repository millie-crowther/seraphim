#include "image.h"

#include <stdexcept>
#include "vk_utils.h"

#include "engine.h"

image_t::image_t(
    uint32_t width, uint32_t height, VkFormat format, 
    VkImageTiling tiling, VkImageUsageFlags usage, 
    VkMemoryPropertyFlags properties, VkImageAspectFlags aspect_flags
){
    is_swapchain = false;
    
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

    layout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkResult result = vkCreateImage(engine_t::get_device(), &image_info, nullptr, &image);
    if (result != VK_SUCCESS){
	throw std::runtime_error("Error: Failed to create image.");
    }

    // allocate memory
    VkMemoryRequirements mem_req;
    vkGetImageMemoryRequirements(engine_t::get_device(), image, &mem_req);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = find_memory_type(mem_req.memoryTypeBits, properties);

    result = vkAllocateMemory(engine_t::get_device(), &alloc_info, nullptr, &memory);
    if (result != VK_SUCCESS){
	    throw std::runtime_error("Error: Failed to allocate image memory.");
    }

    vkBindImageMemory(engine_t::get_device(), image, memory, 0);

    // create image view
    create_image_view(aspect_flags);
}

image_t::image_t(
    VkImage image, VkFormat format, VkImageAspectFlags aspect_flags
){
    is_swapchain = true;
    this->format = format;
    this->image = image;

    create_image_view(aspect_flags);
}

VkFormat
image_t::get_format(){
    return format;
}

void
image_t::create_image_view(VkImageAspectFlags aspect_flags){
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

    VkResult result = vkCreateImageView(engine_t::get_device(), &view_info, nullptr, &image_view);
    if (result != VK_SUCCESS){
	    throw std::runtime_error("Error: Failed to create image view.");
    }
}

image_t::~image_t(){
    vkDestroyImageView(engine_t::get_device(), image_view, nullptr);
    
    if (!is_swapchain){
        vkDestroyImage(engine_t::get_device(), image, nullptr);
        vkFreeMemory(engine_t::get_device(), memory, nullptr);
    }
}

int
image_t::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties){
    VkPhysicalDeviceMemoryProperties memory_prop;
    vkGetPhysicalDeviceMemoryProperties(engine_t::get_physical_device(), &memory_prop);

    for (uint32_t i = 0; i < memory_prop.memoryTypeCount; i++){
        if (
            (type_filter & (1 << i)) &&
            (memory_prop.memoryTypes[i].propertyFlags & properties) == properties
        ){
            return i;
        }
    }

    return -1;
}

VkImage
image_t::get_image(){
    return image;
}

void
image_t::transition_image_layout(VkCommandPool cmd_pool, VkQueue queue, VkImageLayout new_layout){
    vk_utils::single_time_commands(engine_t::get_device(), cmd_pool, queue, [&](VkCommandBuffer cmd){
        VkImageLayout old_layout = layout;      
        
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags src_stage;
        VkPipelineStageFlags dst_stage;


        if (
            old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
            new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        ){
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (
            old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
            new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        ){
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (
            old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
            new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        ){
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
                                  | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            throw std::runtime_error("Error: Unsupported layout transition.");
        }

        if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL){
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            // if has stencil component
            if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT){
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        vkCmdPipelineBarrier(
            cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier
        );
    });

    layout = new_layout;

}

VkImageView
image_t::get_image_view(){
    return image_view;
}

VkFormat
image_t::find_supported_format(
    VkPhysicalDevice physical_device, const std::vector<VkFormat>& candidates, 
    VkImageTiling tiling, VkFormatFeatureFlags features
){
    VkFormatProperties properties;

    for (auto format : candidates){
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &properties);

        if (
            tiling == VK_IMAGE_TILING_LINEAR &&
            (properties.linearTilingFeatures & features) == features
        ){
            return format;
        }

        if (
            tiling == VK_IMAGE_TILING_OPTIMAL &&
            (properties.optimalTilingFeatures & features) == features
        ){
            return format;
        }
    }

    throw std::runtime_error("Error: Failed to find supported image format.");
}

VkFormat
image_t::find_depth_format(VkPhysicalDevice physical_device){
    return find_supported_format(
        physical_device,
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );

}