#include "core/buffer.h"

#include <stdexcept>
#include <iostream>
#include <cstring>
#include "core/blaspheme.h"
#include "core/vk_utils.h"

buffer_t::buffer_t(
    const allocator_t & allocator, uint64_t size, VmaMemoryUsage vma_usage
){
    this->allocator = allocator;
    this->size = size;

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;

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
    VkBuffer dest, uint64_t size, uint64_t offset, VkCommandPool pool, VkQueue queue
){
    if (size == 0){
        return;
    }

    auto cmd = vk_utils::pre_commands(allocator.device, pool, queue);
        VkBufferCopy copy_region = {};
        copy_region.srcOffset = 0;
        copy_region.dstOffset = offset;
        copy_region.size = size;
        
        vkCmdCopyBuffer(cmd, buffer, dest, 1, &copy_region);
    vk_utils::post_commands(allocator.device, pool, queue, cmd);
}

void
buffer_t::copy(
    const void * data, uint64_t size, uint64_t offset, VkCommandPool pool, VkQueue queue
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
            VMA_MEMORY_USAGE_CPU_ONLY
        );

        staging_buffer.copy(data, size, 0, pool, queue);
        staging_buffer.copy_buffer(buffer, size, offset, pool, queue); 
    }
}

void
buffer_t::copy_to_image(
    VkImage image, u32vec2_t offset, u32vec2_t extent, VkCommandPool pool, VkQueue queue
){
    if (extent[0] == 0 || extent[1] == 0){
        return;
    }

    auto cmd = vk_utils::pre_commands(allocator.device, pool, queue);
        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 
            static_cast<int32_t>(offset[0]), 
            static_cast<int32_t>(offset[1]), 
            0 
        };
        region.imageExtent = { extent[0], extent[1], 1 };

        vkCmdCopyBufferToImage(
            cmd, buffer, image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &region
        );
    vk_utils::post_commands(allocator.device, pool, queue, cmd);
}

VkDescriptorBufferInfo 
buffer_t::get_descriptor_info() const {
    VkDescriptorBufferInfo desc_buffer_info = {};
    desc_buffer_info.buffer = buffer;
    desc_buffer_info.offset = 0;
    desc_buffer_info.range  = size;
    return desc_buffer_info;
}
void 
buffer_t::read(void * data, uint64_t size) {
    if (is_host_visible){
        void * mem_map;
        vkMapMemory(allocator.device, memory, 0, size, 0, &mem_map);
            std::memcpy(data, mem_map, size);
        vkUnmapMemory(allocator.device, memory);
    } 
}