#ifndef BUFFER_H
#define BUFFER_H

#include "vk_mem_alloc.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class buffer_t {
private:
    // fields
    bool is_host_visible;

    VmaAllocator allocator;
    VmaAllocation allocation;
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDevice device;

    // helper methods
    void copy_buffer(
	    VkCommandPool command_pool, VkQueue queue, VkBuffer dest, VkDeviceSize size
    );
    VkCommandBuffer pre_commands(VkCommandPool command_pool, VkQueue queue);
    void post_commands(VkCommandPool command_pool, VkQueue queue, VkCommandBuffer command_buffer);

public:
    // constructors and destructors
    buffer_t(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage vma_usage);
    ~buffer_t();

    // public methods
    void copy(VkCommandPool pool, VkQueue queue, void * data, VkDeviceSize size);
    void copy_to_image(VkCommandPool pool, VkQueue queue, VkImage image, uint32_t width, uint32_t height);

    // getters
    VkBuffer get_buffer();
};

#endif
