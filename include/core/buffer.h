#ifndef BUFFER_H
#define BUFFER_H

#include "vk_mem_alloc.h"

#include "core/device.h"
#include "maths/vec.h"


#include <memory>

class buffer_t {
private:
    // fields
    bool is_host_visible;

    VmaAllocator allocator;
    std::shared_ptr<device_t> device;
    VmaAllocation allocation;
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize size;

    // helper methods
    void copy_buffer(
	    VkBuffer dest, uint64_t size, uint64_t offset, VkCommandPool pool, VkQueue queue
    );

public:
    // constructors and destructors
    buffer_t(VmaAllocator allocator, std::shared_ptr<device_t> device, uint64_t size, VmaMemoryUsage vma_usage);
    ~buffer_t();

    // public methods
    void copy(const void * data, uint64_t size, uint64_t offset, VkCommandPool pool, VkQueue queue);
    void copy_to_image(VkImage image, u32vec2_t offset, u32vec2_t extent, VkCommandPool pool, VkQueue queue);
    void read(void * data, uint64_t size);

    // getters
    VkBuffer get_buffer();
    VkDescriptorBufferInfo get_descriptor_info() const;
};

#endif
