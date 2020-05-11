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

public:
    // constructors and destructors
    buffer_t(VmaAllocator allocator, uint32_t binding, std::shared_ptr<device_t> device, uint64_t size, VmaMemoryUsage vma_usage);
    ~buffer_t();

    // public methods
    template<class F>
    void map(uint64_t offset, uint64_t size, const F & f){
        void * memory_map;
        vkMapMemory(device->get_device(), memory, offset, size, 0, &memory_map);
        f(memory_map);
        vkUnmapMemory(device->get_device(), memory);
    }

    template<class T>
    void write(const T & source, uint64_t offset){
        if (source.empty()){
            return;
        }

        uint32_t size = sizeof(typename T::value_type) * source.size();
        map(offset, size, [&](void * memory_map){
            std::memcpy(memory_map, source.data(), size);
        });
    }

    template<class T>
    void read(T & destination, uint64_t offset) {
        if (destination.empty()){
            return;
        }

        uint32_t size = sizeof(typename T::value_type) * destination.size();
        map(offset, size, [&](void * memory_map){
            std::memcpy(destination.data(), memory_map, size);
        });
    }
    
    VkWriteDescriptorSet get_write_descriptor_set(VkDescriptorSet descriptor_set) const;
    VkDescriptorSetLayoutBinding get_descriptor_set_layout_binding() const;

    VkBuffer get_buffer() const;
};

#endif