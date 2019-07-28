#include "core/buffer.h"

#include <stdexcept>
#include <iostream>
#include <cstring>

#include "core/blaspheme.h"
#include "core/command_buffer.h"

buffer_t::buffer_t(
    VmaAllocator allocator, std::shared_ptr<device_t> device, uint64_t size, VmaMemoryUsage vma_usage
){
    this->allocator = allocator;
    this->device = device;
    this->size = size;

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;

    VmaAllocationCreateInfo alloc_create_info = {};
    alloc_create_info.usage = vma_usage;

    VmaAllocationInfo alloc_info = {};
    vmaCreateBuffer(allocator, &buffer_info, &alloc_create_info, &buffer, &allocation, &alloc_info);
   
    memory = alloc_info.deviceMemory;
    
    vkBindBufferMemory(device->get_device(), buffer, memory, 0); 

    is_host_visible = vma_usage != VMA_MEMORY_USAGE_GPU_ONLY;
}

buffer_t::~buffer_t(){
    vmaDestroyBuffer(allocator, buffer, allocation);
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

    command_buffer_t command_buffer(device->get_device(), pool, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    [&](VkCommandBuffer command_buffer){
        VkBufferCopy copy_region = {};
        copy_region.srcOffset = 0;
        copy_region.dstOffset = offset;
        copy_region.size = size;
        
        vkCmdCopyBuffer(command_buffer, buffer, dest, 1, &copy_region);
    });

    auto cmd_buf = command_buffer.get_command_buffer();
    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buf;
    submit_info.pNext = nullptr;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.pWaitDstStageMask = nullptr;
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;
        
    vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue); // TODO: check how bad this is and remove if appropriate
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
        vkMapMemory(device->get_device(), memory, offset, size, 0, &mem_map);
	        std::memcpy(mem_map, data, size);
	    vkUnmapMemory(device->get_device(), memory);
	 
    } else {
        buffer_t staging_buffer(
            allocator, device, size,
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

    command_buffer_t command_buffer(device->get_device(), pool, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    [&](VkCommandBuffer command_buffer){
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
            command_buffer, buffer, image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &region
        );
    });

    auto cmd_buf = command_buffer.get_command_buffer();
    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buf;
    submit_info.pNext = nullptr;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.pWaitDstStageMask = nullptr;
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;
        
    vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue); // TODO: check how bad this is and remove if appropriate
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
        vkMapMemory(device->get_device(), memory, 0, size, 0, &mem_map);
            std::memcpy(data, mem_map, size);
        vkUnmapMemory(device->get_device(), memory);
    } 
}