#include "render/image.h"

#include <stdexcept>

#include "core/engine.h"

image_t::image_t(
    VkImage image, VkFormat format, VkImageAspectFlags aspect_flags
){
    this->format = format;
    this->image = image;

    create_image_view(aspect_flags);
}

VkFormat
swapchain_image_t::get_format(){
    return format;
}

void
swapchain_image_t::create_image_view(VkImageAspectFlags aspect_flags){
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

swapchain_image_t::~swapchain_image_t(){
    vkDestroyImageView(engine_t::get_device(), image_view, nullptr);
}

int
swapchain_image_t::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties){
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
swapchain_image_t::get_image(){
    return image;
}

VkCommandBuffer 
swapchain_image_t::pre_commands(VkCommandPool command_pool, VkQueue queue){
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    VkResult result = vkAllocateCommandBuffers(engine_t::get_device(), &alloc_info, &command_buffer);
    if (result != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to allocate command buffer.");
    }

    VkCommandBufferBeginInfo begin_info;
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pNext = nullptr;
    begin_info.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(command_buffer, &begin_info);
    return command_buffer;
}

void 
swapchain_image_t::post_commands(VkCommandPool command_pool, VkQueue queue, VkCommandBuffer command_buffer){
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    submit_info.pNext = nullptr;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.pWaitDstStageMask = nullptr;
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;
        
    vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(engine_t::get_device(), command_pool, 1, &command_buffer);    
}

void
swapchain_image_t::transition_image_layout(VkCommandPool cmd_pool, VkQueue queue, VkImageLayout new_layout){
    auto cmd = pre_commands(command_pool, queue);
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
    post_commands(command_pool, queue, cmd);

    layout = new_layout;
}

VkImageView
swapchain_image_t::get_image_view(){
    return image_view;
}

VkFormat
swapchain_image_t::find_supported_format(
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
swapchain_image_t::find_depth_format(VkPhysicalDevice physical_device){
    return find_supported_format(
        physical_device,
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}
