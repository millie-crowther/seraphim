#include "buffer.h"

#include "vk_utils.h"
#include <stdexcept>
#include <iostream>
#include <cstring>

buffer_t::buffer_t(
    VkPhysicalDevice physical_device, VkDevice device, VkDeviceSize size, 
    VkBufferUsageFlags usage, VkMemoryPropertyFlags properties
){
    this->device = device;
    this->physical_device = physical_device;

    VkBufferCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &create_info, nullptr, &buffer) != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to create buffer.");
    }

    VkMemoryRequirements memory_req;
    vkGetBufferMemoryRequirements(device, buffer, &memory_req);

    int memory_type = find_memory_type(memory_req.memoryTypeBits, properties);
    if (memory_type == -1){
        throw std::runtime_error("Error: Could not find appropriate memory type.");
    }

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = size;
    alloc_info.memoryTypeIndex = memory_type;

    if (vkAllocateMemory(device, &alloc_info, nullptr, &memory) != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to allocate buffer memory.");
    }
    vkBindBufferMemory(device, buffer, memory, 0);   
   
    is_host_visible = (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT & properties) != 0;
}

buffer_t::~buffer_t(){
    vkDestroyBuffer(device, buffer, nullptr);
    vkFreeMemory(device, memory, nullptr);
}

int
buffer_t::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties){
    VkPhysicalDeviceMemoryProperties memory_prop;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_prop);

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
buffer_t::get_buffer(){
    return buffer;
}

void
buffer_t::copy_buffer(
    VkCommandPool command_pool, VkQueue queue, VkBuffer dest, VkDeviceSize size
){
    vk_utils::single_time_commands(device, command_pool, queue, [&](VkCommandBuffer cmd){
        VkBufferCopy copy_region = {};
        copy_region.srcOffset = 0;
        copy_region.dstOffset = 0;
        copy_region.size = size;

        vkCmdCopyBuffer(cmd, buffer, dest, 1, &copy_region);
    });
}

VkDeviceMemory
buffer_t::get_memory(){
    return memory;
}

void
buffer_t::copy(
    VkCommandPool command_pool, VkQueue queue, void * data, VkDeviceSize size
){
    if (is_host_visible){
	void * mem_map;
        vkMapMemory(device, memory, 0, size, 0, &mem_map);
	    std::memcpy(mem_map, data, size);
	vkUnmapMemory(device, memory);
	 
    } else {
        buffer_t staging_buffer(
            physical_device,
            device,
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        staging_buffer.copy(command_pool, queue, data, size);
        staging_buffer.copy_buffer(command_pool, queue, buffer, size); 
    }
}
