#include "core/buffer.h"

#include <stdexcept>
#include <iostream>
#include <cstring>
#include "core/blaspheme.h"
#include "core/vk_utils.h"

buffer_t::buffer_t(
    const allocator_t & allocator, uint64_t size, VkBufferUsageFlags usage, VmaMemoryUsage vma_usage
){
    this->allocator = allocator;

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;

    VmaAllocationCreateInfo alloc_create_info = {};
    alloc_create_info.usage = vma_usage;

    VmaAllocationInfo alloc_info = {};
    vmaCreateBuffer(allocator.vma_allocator, &buffer_info, &alloc_create_info, &buffer, &allocation, &alloc_info);
   
    memory = alloc_info.deviceMemory;
    
    vkBindBufferMemory(allocator.device, buffer, memory, 0); 

    is_host_visible = vma_usage != VMA_MEMORY_USAGE_GPU_ONLY;
}

buffer_t::~buffer_t(){
    vmaDestroyBuffer(allocator.vma_allocator, buffer, allocation);
}

VkBuffer
buffer_t::get_buffer(){
    return buffer;
}

void
buffer_t::copy_buffer(
    VkBuffer dest, uint64_t size, uint64_t offset
){
    if (size == 0){
        return;
    }

    auto cmd = vk_utils::pre_commands(allocator.device, allocator.pool, allocator.queue);
        VkBufferCopy copy_region = {};
        copy_region.srcOffset = 0;
        copy_region.dstOffset = offset;
        copy_region.size = size;
        
        vkCmdCopyBuffer(cmd, buffer, dest, 1, &copy_region);
    vk_utils::post_commands(allocator.device, allocator.pool, allocator.queue, cmd);
}

void
buffer_t::copy(
    void * data, uint64_t size, uint64_t offset
){
    if (size == 0){
        return;
    }

    if (is_host_visible){
	    void * mem_map;
        vkMapMemory(allocator.device, memory, offset, size, 0, &mem_map);
	        std::memcpy(mem_map, data, size);
	    vkUnmapMemory(allocator.device, memory);
	 
    } else {
        buffer_t staging_buffer(
            allocator, size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_CPU_ONLY
        );

        staging_buffer.copy(data, size, 0);
        staging_buffer.copy_buffer(buffer, size, offset); 
    }
}

void
buffer_t::copy_to_image(
    VkImage image, uint32_t width, uint32_t height
){
    if (width <= 0 || height <= 0){
        return;
    }

    auto cmd = vk_utils::pre_commands(allocator.device, allocator.pool, allocator.queue);
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
    vk_utils::post_commands(allocator.device, allocator.pool, allocator.queue, cmd);
}
