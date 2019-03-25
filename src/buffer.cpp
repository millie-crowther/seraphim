#include "buffer.h"

#include <stdexcept>
#include <iostream>
#include <cstring>
#include "core/blaspheme.h"

raw_buffer_t::raw_buffer_t(
    VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties
){
    VkBufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(blaspheme_t::get_device(), &create_info, nullptr, &buffer) != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to create buffer.");
    }

    VkMemoryRequirements memory_req;
    vkGetBufferMemoryRequirements(blaspheme_t::get_device(), buffer, &memory_req);

    int memory_type = find_memory_type(memory_req.memoryTypeBits, properties);
    if (memory_type == -1){
        throw std::runtime_error("Error: Could not find appropriate memory type.");
    }

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = size;
    alloc_info.memoryTypeIndex = memory_type;

    if (vkAllocateMemory(blaspheme_t::get_device(), &alloc_info, nullptr, &memory) != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to allocate buffer memory.");
    }
    vkBindBufferMemory(blaspheme_t::get_device(), buffer, memory, 0);   
   
    is_host_visible = (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT & properties) != 0;
}

raw_buffer_t::~raw_buffer_t(){
    vkDestroyBuffer(blaspheme_t::get_device(), buffer, nullptr);
    vkFreeMemory(blaspheme_t::get_device(), memory, nullptr);
}

VkCommandBuffer 
raw_buffer_t::pre_commands(VkCommandPool command_pool, VkQueue queue){
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
raw_buffer_t::post_commands(VkCommandPool command_pool, VkQueue queue, VkCommandBuffer command_buffer){
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

int
raw_buffer_t::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties){
    VkPhysicalDeviceMemoryProperties memory_prop;
    vkGetPhysicalDeviceMemoryProperties(blaspheme_t::get_physical_device(), &memory_prop);

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

VkBuffer
raw_buffer_t::get_buffer(){
    return buffer;
}

void
raw_buffer_t::copy_buffer(
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

VkDeviceMemory
raw_buffer_t::get_memory(){
    return memory;
}

void
raw_buffer_t::copy(
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
        raw_buffer_t staging_buffer(
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        staging_buffer.copy(command_pool, queue, data, size);
        staging_buffer.copy_buffer(command_pool, queue, buffer, size); 
    }
}

void
raw_buffer_t::copy_to_image(
    VkCommandPool pool, VkQueue queue, VkImage image, int width, int height
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

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            (uint32_t) width,
            (uint32_t) height,
            1
        };

        vkCmdCopyBufferToImage(
            cmd, buffer, image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &region
        );
    post_commands(pool, queue, cmd);
}
