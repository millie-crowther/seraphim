#include "vk_utils.h"

#include "engine.h"

namespace vk_utils {
    void
    single_time_commands(
        VkCommandPool command_pool, VkQueue queue, 
        std::function<void(VkCommandBuffer)> commands
    ){
        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = command_pool;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer;
        VkResult result = vkAllocateCommandBuffers(engine_t::get_device(), &alloc_info, &command_buffer);
        if (result != VK_SUCCESS){
            throw std::runtime_error("Error: Failed to allocate command buffer.");
        }

        VkCommandBufferBeginInfo begin_info;
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        begin_info.pNext = nullptr;
        begin_info.pInheritanceInfo = nullptr;

        vkBeginCommandBuffer(command_buffer, &begin_info);
            commands(command_buffer);
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

        vkFreeCommandBuffers(engine_t::get_device(), command_pool, 1, &command_buffer);    
    }
}