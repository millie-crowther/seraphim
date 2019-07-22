#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

template<class F>
VkCommandBuffer 
create_command_buffer(VkDevice device, VkCommandPool pool, VkCommandBufferUsageFlags usage, const F & f){
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    VkResult result = vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);
    if (result != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to allocate command buffer.");
    }

    VkCommandBufferBeginInfo begin_info;
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = usage;
    begin_info.pNext = nullptr;
    begin_info.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to begin command buffer");
    }

    f(command_buffer);

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to end command buffer");
    }

    return command_buffer;
}

#endif