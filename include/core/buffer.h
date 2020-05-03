#ifndef BUFFER_H
#define BUFFER_H

#include "vk_mem_alloc.h"

#include <cstring>
#include <memory>

#include "core/command.h"
#include "core/device.h"
#include "maths/vec.h"

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

    template<typename collection_t>
    void map(const collection_t & c, uint64_t offset, bool is_write){
        if (c.empty()){
            return;
        }

        void * memory_map;
        vkMapMemory(
            device->get_device(), memory, offset, 
            sizeof(typename collection_t::value_type) * c.size(), 0, &memory_map
        );

        void * from = is_write ? memory_map          : ((void *) c.data());
        void * to   = is_write ? ((void *) c.data()) : memory_map;
        std::memcpy(from, to, sizeof(typename collection_t::value_type) * c.size());
        vkUnmapMemory(device->get_device(), memory);
    }

public:
    // constructors and destructors
    buffer_t(VmaAllocator allocator, uint32_t binding, std::shared_ptr<device_t> device, uint64_t size, VmaMemoryUsage vma_usage);
    ~buffer_t();

    // public methods
    template<typename collection_t>
    void write(const collection_t & source, uint64_t offset){
        map(source, offset, true);
    }

    template<class collection_t>
    void read(const collection_t & destination, uint64_t offset) {
        map(destination, offset, false);
    }

    VkWriteDescriptorSet get_write_descriptor_set(VkDescriptorSet descriptor_set) const;
    VkDescriptorSetLayoutBinding get_descriptor_set_layout_binding() const;
};

#endif