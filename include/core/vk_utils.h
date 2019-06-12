#ifndef VK_UTILS
#define VK_UTILS

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vk_utils {
    VkCommandBuffer pre_commands(VkDevice device, VkCommandPool command_pool, VkQueue queue);
    void post_commands(VkDevice device, VkCommandPool command_pool, VkQueue queue, VkCommandBuffer command_buffer);
}

#endif
