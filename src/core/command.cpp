#include "core/command.h"

using namespace srph;

command_buffer_t::~command_buffer_t(){
    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

void command_buffer_t::submit(VkSemaphore wait_sema, VkSemaphore signal_sema, VkFence fence, VkPipelineStageFlags stage){
    VkSubmitInfo submit_info = {};
    submit_info.pWaitDstStageMask = &stage;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    
    submit_info.waitSemaphoreCount = wait_sema == VK_NULL_HANDLE ? 0 : 1;
    submit_info.pWaitSemaphores = &wait_sema;
    submit_info.signalSemaphoreCount = signal_sema == VK_NULL_HANDLE ? 0 : 1;
    submit_info.pSignalSemaphores = &signal_sema;

    if (vkQueueSubmit(queue, 1, &submit_info, fence) != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to submit command buffer to queue.");
    }
}

command_pool_t::command_pool_t(VkDevice device, uint32_t queue_family){
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

command_pool_t::~command_pool_t(){
    vkDestroyCommandPool(device, command_pool, nullptr);
}
