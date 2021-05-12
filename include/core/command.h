#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>
#include <stdexcept>

typedef struct command_buffer_t {
    VkCommandBuffer command_buffer;
    VkDevice device;
    VkCommandPool command_pool;
    VkQueue queue;
    bool is_one_time;
} command_buffer_t;

struct command_pool_t {
    VkDevice device;
    VkCommandPool command_pool;
    VkQueue queue;

    command_pool_t(VkDevice device, uint32_t queue_family);

    ~command_pool_t();
};

void command_buffer_destroy(command_buffer_t * command_buffer);
void
command_buffer_submit(command_buffer_t *command_buffer, VkSemaphore wait_sema, VkSemaphore signal_sema, VkFence fence,
                      VkPipelineStageFlags stage);

void command_buffer_begin_buffer(command_pool_t * pool, command_buffer_t *buffer, bool is_one_time);
void command_buffer_end(command_buffer_t *buffer);

#endif
