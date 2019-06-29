#include "render/image.h"

#include <stdexcept>

#include "core/blaspheme.h"
#include "core/vk_utils.h"

image_t::image_t(
    const allocator_t & allocator,
    u32vec2_t & size, VkImageUsageFlags usage, 
    VmaMemoryUsage vma_usage
){
    is_swapchain = false;
    
    this->format = VK_FORMAT_R8G8B8A8_UNORM;
    this->allocator = allocator;

    layout = VK_IMAGE_LAYOUT_UNDEFINED;

    // create image
    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = size[0];
    image_info.extent.height = size[1];
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = layout;
    image_info.usage = usage;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


    // VmaAllocationCreateInfo alloc_create_info = {};
    // alloc_create_info.usage = vma_usage; 

    // VmaAllocation allocation;
    // VmaAllocationInfo alloc_info;

    // TODO: Use VMA to do the allocation. not sure why below approach doesn't work.
    //       screen goes black if you try this way
    //  
    //
    // VkResult result = vmaCreateImage(
    //     allocator, &image_info, &alloc_create_info,
    //     &image, &allocation, &alloc_info
    // );

    // if (result != VK_SUCCESS){
	//     throw std::runtime_error("Error: Failed to create image.");
    // }

    // memory = alloc_info.deviceMemory;


    // // allocate memory 
    VkResult result = vkCreateImage(allocator.device, &image_info, nullptr, &image);
    if (result != VK_SUCCESS){
	    throw std::runtime_error("Error: Failed to create image.");
    }
    VkMemoryRequirements mem_req;
    vkGetImageMemoryRequirements(allocator.device, image, &mem_req);

    VkMemoryAllocateInfo mem_alloc_info = {};
    mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc_info.allocationSize = mem_req.size;
    auto properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    mem_alloc_info.memoryTypeIndex = find_memory_type(mem_req.memoryTypeBits, properties);

    result = vkAllocateMemory(allocator.device, &mem_alloc_info, nullptr, &memory);
    if (result != VK_SUCCESS){
	    throw std::runtime_error("Error: Failed to allocate image memory.");
    }

    result = vkBindImageMemory(allocator.device, image, memory, 0);

    if (result != VK_SUCCESS){
	    throw std::runtime_error("Error: Failed to create image.");
    }

    // create image view
    create_image_view();
}

image_t::image_t(
    const allocator_t & allocator,
    VkImage image, VkFormat format
){
    is_swapchain = true;
    this->allocator = allocator;
    this->format = format;
    this->image = image;

    create_image_view();
}

VkFormat
image_t::get_format(){
    return format;
}

VkImageLayout
image_t::get_image_layout() const {
    return layout;
}

void
image_t::create_image_view(){
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkResult result = vkCreateImageView(allocator.device, &view_info, nullptr, &image_view);
    if (result != VK_SUCCESS){
	    throw std::runtime_error("Error: Failed to create image view.");
    }
}

image_t::~image_t(){
    vkDestroyImageView(allocator.device, image_view, nullptr);
    
    if (!is_swapchain){
        vkDestroyImage(allocator.device, image, nullptr);
        vkFreeMemory(allocator.device, memory, nullptr);
        // vmaDestroyImage(allocator, image, allocation);
    }
}

int
image_t::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties){
    VkPhysicalDeviceMemoryProperties memory_prop;
    vkGetPhysicalDeviceMemoryProperties(allocator.physical_device, &memory_prop);

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
image_t::transition_image_layout(VkImageLayout new_layout){
    auto cmd = vk_utils::pre_commands(allocator.device, allocator.pool, allocator.queue);
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
    vk_utils::post_commands(allocator.device, allocator.pool, allocator.queue, cmd);

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
