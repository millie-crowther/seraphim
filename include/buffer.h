#ifndef BUFFER_H
#define BUFFER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>

class raw_buffer_t {
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
    VkCommandBuffer pre_commands(VkCommandPool command_pool, VkQueue queue);
    void post_commands(VkCommandPool command_pool, VkQueue queue, VkCommandBuffer command_buffer);

public:
    // constructors and destructors
    raw_buffer_t(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~raw_buffer_t();

    // public methods
    void copy(VkCommandPool pool, VkQueue queue, void * data, VkDeviceSize size);
    void copy_to_image(VkCommandPool pool, VkQueue queue, VkImage image, int width, int height);

    // getters
    VkBuffer get_buffer();
    VkDeviceMemory get_memory();
    bool is_valid();
};

typedef std::shared_ptr<raw_buffer_t> buffer_t;

#endif
