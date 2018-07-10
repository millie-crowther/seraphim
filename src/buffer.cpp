#include "buffer.h"

#include "vk_utils.h"
#include <stdexcept>

buffer_t::buffer_t(
    VkPhysicalDevice physical_device, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties
){
    this->device = device;

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

    int memory_type = find_memory_type(physical_device, memory_req.memoryTypeBits, properties);
    if (memory_type == -1){
        throw std::runtime_error("Error: Couldn't find appropriate memory type.");
    }

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = memory_req.size;
    alloc_info.memoryTypeIndex = memory_type;

    if (vkAllocateMemory(device, &alloc_info, nullptr, &memory) != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to allocate memory for buffer.");
    }

    vkBindBufferMemory(device, buffer, memory, 0);   

    is_valid = true;
}

buffer_t::~buffer_t(){
    if (is_valid){
        destroy();
    }
}

int
buffer_t::find_memory_type(
    VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties
){
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

void
buffer_t::copy(VkCommandPool command_pool, VkQueue queue, VkBuffer dest, VkDeviceSize size){
    if (!is_valid){
        throw std::runtime_error("Error: Tried to copy a buffer that was already deleted.");
    }

    vk_utils::single_time_commands(device, command_pool, queue, [&](VkCommandBuffer cmd){
        VkBufferCopy copy_region = {};
        copy_region.srcOffset = 0;
        copy_region.dstOffset = 0;
        copy_region.size = size;

        vkCmdCopyBuffer(cmd, buffer, dest, 1, &copy_region);
    });
}

void
buffer_t::destroy(){
   if (is_valid){
       vkDestroyBuffer(device, buffer, nullptr);
       vkFreeMemory(device, memory, nullptr);
       is_valid = false;
   }
}
