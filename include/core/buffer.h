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
	    VkCommandPool command_pool, VkQueue queue, VkBuffer dest, uint64_t size, uint64_t offset
    );

public:
    // constructors and destructors
    buffer_t(VmaAllocator allocator, uint64_t size, VkBufferUsageFlags usage, VmaMemoryUsage vma_usage);
    ~buffer_t();

    // public methods
    void copy(VkCommandPool pool, VkQueue queue, void * data, uint64_t size, uint64_t offset);
    void copy_to_image(VkCommandPool pool, VkQueue queue, VkImage image, uint32_t width, uint32_t height);

    // getters
    VkBuffer get_buffer();
};

#endif
