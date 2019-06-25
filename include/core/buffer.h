#ifndef BUFFER_H
#define BUFFER_H

#include "core/allocator.h"

#include "vk_mem_alloc.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class buffer_t {
private:
    // fields
    bool is_host_visible;

    allocator_t allocator; // TODO: maybe dont keep a copy
    VmaAllocation allocation;
    VkBuffer buffer;
    VkDeviceMemory memory;

    // helper methods
    void copy_buffer(
	    VkBuffer dest, uint64_t size, uint64_t offset
    );

public:
    // constructors and destructors
    buffer_t(const allocator_t & allocator, uint64_t size, VkBufferUsageFlags usage, VmaMemoryUsage vma_usage);
    ~buffer_t();

    // public methods
    void copy(const void * data, uint64_t size, uint64_t offset);
    void copy_to_image(VkImage image, int x, int y, uint32_t width, uint32_t height);

    // getters
    VkBuffer get_buffer();
};

#endif
