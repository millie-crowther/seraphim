#ifndef BUFFER_H
#define BUFFER_H

#include "vk_mem_alloc.h"

#include <cstring>
#include <memory>

#include "core/command.h"
#include "core/device.h"
#include "maths/vec.h"
#include "render/texture.h"

class buffer_t {
private:
    VmaAllocator allocator;
    std::shared_ptr<device_t> device;
    VmaAllocation allocation;
    VkBuffer buffer;
    VkDeviceMemory memory;
    uint32_t size;
    uint32_t binding;
    VkDescriptorBufferInfo desc_buffer_info;

    template<typename T>
    void map(const T & x, uint64_t offset, bool is_write){
        if (x.empty()){
            return;
        }

        void * memory_map;
        vkMapMemory(
            device->get_device(), memory, offset, 
            sizeof(typename T::value_type) * x.size(), 0, &memory_map
        );

        void * from = is_write ? memory_map          : ((void *) x.data());
        void * to   = is_write ? ((void *) x.data()) : memory_map;
        std::memcpy(from, to, sizeof(typename T::value_type) * x.size());
        vkUnmapMemory(device->get_device(), memory);
    }

public:
    // constructors and destructors
    buffer_t(VmaAllocator allocator, uint32_t binding, std::shared_ptr<device_t> device, uint64_t size, VmaMemoryUsage vma_usage);
    ~buffer_t();

    // public methods
    template<typename T>
    void write(const T & source, uint64_t offset){
        map(source, offset, true);
    }

    template<class T>
    void read(const T & destination, uint64_t offset) {
        map(destination, offset, false);
    }
    
    void copy_to_image(const command_pool_t & command_pool, texture_t & texture, uint32_t width, uint32_t height);

    VkWriteDescriptorSet get_write_descriptor_set(VkDescriptorSet descriptor_set) const;
    VkDescriptorSetLayoutBinding get_descriptor_set_layout_binding() const;
};

#endif