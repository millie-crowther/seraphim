#include "buffer.h"

#include <stdexcept>
#include <iostream>
#include <cstring>
#include "core/blaspheme.h"

buffer_t::buffer_t(
    VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage vma_usage
){
    this->allocator = allocator;

    

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;

    VmaAllocationCreateInfo alloc_create_info = {};
    alloc_create_info.usage = vma_usage;

    vmaCreateBuffer(allocator, &buffer_info, &alloc_create_info, &buffer, &allocation, nullptr);
   
    VmaAllocationInfo alloc_info = {};
    vmaGetAllocationInfo(allocator, allocation, &alloc_info);
    memory = alloc_info.deviceMemory;
    
    vkBindBufferMemory(blaspheme_t::get_device(), buffer, memory, 0); 

    is_host_visible = vma_usage != VMA_MEMORY_USAGE_GPU_ONLY;
}

buffer_t::~buffer_t(){
    vmaDestroyBuffer(allocator, buffer, allocation);
}

VkCommandBuffer 
buffer_t::pre_commands(VkCommandPool command_pool, VkQueue queue){
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    VkResult result = vkAllocateCommandBuffers(blaspheme_t::get_device(), &alloc_info, &command_buffer);
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
buffer_t::post_commands(VkCommandPool command_pool, VkQueue queue, VkCommandBuffer command_buffer){
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

    vkFreeCommandBuffers(blaspheme_t::get_device(), command_pool, 1, &command_buffer);    
}

VkBuffer
buffer_t::get_buffer(){
    return buffer;
}

void
buffer_t::copy_buffer(
    VkCommandPool command_pool, VkQueue queue, VkBuffer dest, VkDeviceSize size
){
    if (size == 0){
        return;
    }

    auto cmd = pre_commands(command_pool, queue);
        VkBufferCopy copy_region = {};
        copy_region.srcOffset = 0;
        copy_region.dstOffset = 0;
        copy_region.size = size;
        
        vkCmdCopyBuffer(cmd, buffer, dest, 1, &copy_region);
    post_commands(command_pool, queue, cmd);
}

void
buffer_t::copy(
    VkCommandPool command_pool, VkQueue queue, void * data, VkDeviceSize size
){
    if (size == 0){
        return;
    }

    if (is_host_visible){
	    void * mem_map;
        vkMapMemory(blaspheme_t::get_device(), memory, 0, size, 0, &mem_map);
	        std::memcpy(mem_map, data, size);
	    vkUnmapMemory(blaspheme_t::get_device(), memory);
	 
    } else {
        buffer_t staging_buffer(
            allocator, size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_CPU_ONLY
        );

        staging_buffer.copy(command_pool, queue, data, size);
        staging_buffer.copy_buffer(command_pool, queue, buffer, size); 
    }
}

void
buffer_t::copy_to_image(
    VkCommandPool pool, VkQueue queue, VkImage image, uint32_t width, uint32_t height
){
    if (width <= 0 || height <= 0){
        return;
    }

    auto cmd = pre_commands(pool, queue);
        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(
            cmd, buffer, image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &region
        );
    post_commands(pool, queue, cmd);
}
