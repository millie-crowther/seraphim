#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

class command_buffer_t {
private:
    VkCommandBuffer command_buffer;
    VkDevice device;
    VkCommandPool command_pool;

public:
    template<class F>
    command_buffer_t(VkDevice device, VkCommandPool command_pool, VkCommandBufferUsageFlags usage, const F & f){
        this->device = device;
        this->command_pool = command_pool;

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = command_pool;
        alloc_info.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(device, &alloc_info, &command_buffer) != VK_SUCCESS){
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
    } 

    ~command_buffer_t(){
        vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
    }

    VkCommandBuffer get_command_buffer() const {
        return command_buffer;
    }
};

#endif