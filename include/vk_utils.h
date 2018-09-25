#ifndef BLASPHEME_VK_UTILS_H
#define BLASPHEME_VK_UTILS_H

#include <functional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vk_utils {
    void single_time_commands(
       VkCommandPool command_pool, VkQueue queue, 
       std::function<void(VkCommandBuffer)> commands
    );
}

#endif
