#ifndef BUFFER_H
#define BUFER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class buffer_t {
private:
    // fields
    bool is_val;
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDevice device;

    // helper methods
    int find_memory_type(
        VkPhysicalDevice physical_device, uint32_t memory_type_mask, VkMemoryPropertyFlags properties
    );

public:
    // constructors and destructors
    buffer_t();
    bool initialise(
        VkPhysicalDevice physical_device, VkDevice device, VkDeviceSize size, 
        VkBufferUsageFlags usage, VkMemoryPropertyFlags properties
    );
    ~buffer_t();

    // public methods
    void copy(VkCommandPool command_pool, VkQueue queue, const buffer_t& dest, VkDeviceSize size);
    void destroy();

    // getters
    VkBuffer get_buffer();
    VkDeviceMemory get_memory();
    bool is_valid();
};

#endif
