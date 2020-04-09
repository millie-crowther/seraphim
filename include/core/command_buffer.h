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
    VkQueue queue;

public:
    template<class F>
    command_buffer_t(VkDevice device, VkCommandPool command_pool, VkQueue queue, VkCommandBufferUsageFlags usage, const F & f){
        this->device = device;
        this->command_pool = command_pool;
        this->queue = queue;

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

    void submit(VkSemaphore wait_sema, VkSemaphore signal_sema, VkFence fence, VkPipelineStageFlags stage){
        VkSubmitInfo submit_info = {};
        submit_info.pWaitDstStageMask = &stage;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        
        submit_info.waitSemaphoreCount = wait_sema == VK_NULL_HANDLE ? 0 : 1;
        submit_info.pWaitSemaphores = &wait_sema;
        submit_info.signalSemaphoreCount = signal_sema == VK_NULL_HANDLE ? 0 : 1;
        submit_info.pSignalSemaphores = &signal_sema;

        vkQueueSubmit(queue, 1, &submit_info, fence);
    }
};

class command_pool_t {
private:
    VkDevice device;
    VkCommandPool command_pool;
    VkQueue queue;

public:
    command_pool_t(VkDevice device, uint32_t queue_family){
        this->device = device;

        VkCommandPoolCreateInfo command_pool_info = {};
        command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_info.queueFamilyIndex = queue_family;
        command_pool_info.flags = 0;

        if (vkCreateCommandPool(device, &command_pool_info, nullptr, &command_pool) != VK_SUCCESS){
            throw std::runtime_error("Error: failed to create command pool.");
        }    

        vkGetDeviceQueue(device, queue_family, 0, &queue);
    }

    ~command_pool_t(){
        vkDestroyCommandPool(device, command_pool, nullptr);
    }

    template<class F>
    std::shared_ptr<command_buffer_t> create_command_buffer(VkCommandBufferUsageFlags usage, const F & f) const {
        return std::make_shared<command_buffer_t>(device, command_pool, queue, usage, f);
    }
};

#endif