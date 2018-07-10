#ifndef BUFFER_H
#define BUFER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class buffer_t {
private:
    // fields
    bool is_valid;
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDevice device;

    // helper methods
    int find_memory_type(
        VkPhysicalDevice physical_device, uint32_t memory_type_mask, VkMemoryPropertyFlags properties
    );

public:
    // constructors and destructors
    buffer_t(
        VkPhysicalDevice physical_device, VkDevice device, VkDeviceSize size, 
        VkBufferUsageFlags usage, VkMemoryPropertyFlags properties
    );
    ~buffer_t();

    // public methods
    void copy(VkCommandPool command_pool, VkQueue queue, VkBuffer dest, VkDeviceSize size);
    void destroy();
};

#endif
