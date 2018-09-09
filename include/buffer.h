#ifndef BUFFER_H
#define BUFFER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class buffer_t {
private:
    // fields
    bool is_host_visible;
    VkBuffer buffer;
    VkDeviceMemory memory;

    // helper methods
    int find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties);
    void copy_buffer(
	    VkCommandPool command_pool, VkQueue queue, VkBuffer dest, VkDeviceSize size
    );

public:
    // constructors and destructors
    buffer_t(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~buffer_t();

    // public methods
    void copy(VkCommandPool pool, VkQueue queue, void * data, VkDeviceSize size);
    void copy_to_image(VkCommandPool pool, VkQueue queue, VkImage image, int width, int height);

    // getters
    VkBuffer get_buffer();
    VkDeviceMemory get_memory();
    bool is_valid();
};

#endif
