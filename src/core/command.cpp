#include "core/command.h"

void command_buffer_submit(command_buffer_t  * command_buffer, VkSemaphore wait_sema, VkSemaphore signal_sema,
                              VkFence fence, VkPipelineStageFlags stage) {
    VkSubmitInfo submit_info = {};
    submit_info.pWaitDstStageMask = &stage;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer->command_buffer;

    submit_info.waitSemaphoreCount = wait_sema == VK_NULL_HANDLE ? 0 : 1;
    submit_info.pWaitSemaphores = &wait_sema;
    submit_info.signalSemaphoreCount = signal_sema == VK_NULL_HANDLE ? 0 : 1;
    submit_info.pSignalSemaphores = &signal_sema;

    if (vkQueueSubmit(command_buffer->queue, 1, &submit_info, fence) != VK_SUCCESS) {
        throw std::runtime_error("Error: Failed to submit command buffer to queue.");
    }

    if (command_buffer->is_one_time){
        command_buffer_destroy(command_buffer);
    }
}

void command_buffer_begin_buffer(command_pool_t *pool, command_buffer_t *buffer, bool is_one_time) {
    buffer->device = pool->device;
    buffer->command_pool = pool->command_pool;
    buffer->queue = pool->queue;
    buffer->is_one_time = is_one_time;
    VkCommandBufferUsageFlags usage = is_one_time ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = buffer->command_pool;
    alloc_info.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(buffer->device, &alloc_info, &buffer->command_buffer) !=
        VK_SUCCESS) {
        throw std::runtime_error("Error: Failed to allocate command buffer.");
    }

    VkCommandBufferBeginInfo begin_info;
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = usage;
    begin_info.pNext = nullptr;
    begin_info.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(buffer->command_buffer, &begin_info) != VK_SUCCESS) {
        throw std::runtime_error("Error: Failed to begin command buffer");
    }
}

void command_buffer_end(command_buffer_t *buffer) {
    if (vkEndCommandBuffer(buffer->command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("Error: Failed to end command buffer");
    }
}

void command_buffer_destroy(command_buffer_t *command_buffer) {
    vkFreeCommandBuffers(command_buffer->device, command_buffer->command_pool, 1, &command_buffer->command_buffer);
}

command_pool_t::command_pool_t(VkDevice device, uint32_t queue_family) {
    this->device = device;

    VkCommandPoolCreateInfo command_pool_info = {};
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.queueFamilyIndex = queue_family;
    command_pool_info.flags = 0;

    if (vkCreateCommandPool(device, &command_pool_info, nullptr, &command_pool) !=
        VK_SUCCESS) {
        throw std::runtime_error("Error: failed to create command pool.");
    }

    vkGetDeviceQueue(device, queue_family, 0, &queue);
}

command_pool_t::~command_pool_t() {
    vkDestroyCommandPool(device, command_pool, nullptr);
}