#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>
#include <stdexcept>

namespace srph {
struct command_buffer_t {
    VkCommandBuffer command_buffer;
    VkDevice device;
    VkCommandPool command_pool;
    VkQueue queue;

    template <class F>
    command_buffer_t(VkDevice device, VkCommandPool command_pool, VkQueue queue,
                     VkCommandBufferUsageFlags usage, const F &f) {
        this->device = device;
        this->command_pool = command_pool;
        this->queue = queue;

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = command_pool;
        alloc_info.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(device, &alloc_info, &command_buffer) !=
            VK_SUCCESS) {
            throw std::runtime_error("Error: Failed to allocate command buffer.");
        }

        VkCommandBufferBeginInfo begin_info;
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = usage;
        begin_info.pNext = nullptr;
        begin_info.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
            throw std::runtime_error("Error: Failed to begin command buffer");
        }

        f(command_buffer);

        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
            throw std::runtime_error("Error: Failed to end command buffer");
        }
    }

    ~command_buffer_t();

    void submit(VkSemaphore wait_sema, VkSemaphore signal_sema, VkFence fence,
                VkPipelineStageFlags stage);
};

struct command_pool_t {
    VkDevice device;
    VkCommandPool command_pool;
    VkQueue queue;

    command_pool_t(VkDevice device, uint32_t queue_family);

    ~command_pool_t();

    template <class F>
    std::shared_ptr<command_buffer_t> one_time_buffer(const F &f) const {
        return std::make_shared<command_buffer_t>(
            device, command_pool, queue, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            f);
    }

    template <class F>
    std::shared_ptr<command_buffer_t> reusable_buffer(const F &f) const {
        return std::make_shared<command_buffer_t>(
            device, command_pool, queue,
            VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, f);
    }
};
} // namespace srph

#endif
