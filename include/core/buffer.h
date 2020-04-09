#ifndef BUFFER_H
#define BUFFER_H

#include "vk_mem_alloc.h"

#include <cstring>
#include <memory>

#include "core/command_buffer.h"
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
    void * memory_map;

public:
    // constructors and destructors
    buffer_t(VmaAllocator allocator, uint32_t binding, std::shared_ptr<device_t> device, uint64_t size, VmaMemoryUsage vma_usage){
        this->allocator = allocator;
        this->device = device;
        this->size = size;
        this->binding = binding;

        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;

        VmaAllocationCreateInfo alloc_create_info = {};
        alloc_create_info.usage = vma_usage;

        VmaAllocationInfo alloc_info = {};
        vmaCreateBuffer(allocator, &buffer_info, &alloc_create_info, &buffer, &allocation, &alloc_info);
    
        memory = alloc_info.deviceMemory;
        
        vkBindBufferMemory(device->get_device(), buffer, memory, 0); 

        desc_buffer_info = {};
        desc_buffer_info.buffer = buffer;
        desc_buffer_info.offset = 0;
        desc_buffer_info.range  = size;
    }

    ~buffer_t(){
        vmaDestroyBuffer(allocator, buffer, allocation);
    }

    // public methods
    template<typename collection_t>
    void write(const collection_t & source, uint64_t offset){
        if (source.empty()){
            return;
        }

        vkMapMemory(
            device->get_device(), memory, sizeof(typename collection_t::value_type) * offset, 
            sizeof(typename collection_t::value_type) * source.size(), 0, &memory_map
        );
        std::memcpy(memory_map, source.data(), sizeof(typename collection_t::value_type) * source.size());
        vkUnmapMemory(device->get_device(), memory);
    }

    template<class collection_t>
    void read(collection_t & destination) {
        if (destination.empty()){
            return;
        }

        vkMapMemory(
            device->get_device(), memory, 0, 
            sizeof(typename collection_t::value_type) * destination.size(), 0, &memory_map
        );
        std::memcpy(destination.data(), memory_map, sizeof(typename collection_t::value_type) * destination.size());
        vkUnmapMemory(device->get_device(), memory);
    }

    VkWriteDescriptorSet get_write_descriptor_set(VkDescriptorSet descriptor_set) const {
        VkWriteDescriptorSet write_desc_set = {};
        write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc_set.pNext = nullptr;
        write_desc_set.dstArrayElement = 0;
        write_desc_set.descriptorCount = 1;
        write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write_desc_set.pImageInfo = nullptr;
        write_desc_set.pTexelBufferView = nullptr;
        write_desc_set.dstSet = descriptor_set;
        write_desc_set.dstBinding = binding;
        write_desc_set.pBufferInfo = &desc_buffer_info;
        return write_desc_set;
    }

    VkDescriptorSetLayoutBinding get_descriptor_set_layout_binding() const {
        VkDescriptorSetLayoutBinding layout_binding = {};
        layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layout_binding.descriptorCount = 1;
        layout_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        layout_binding.pImmutableSamplers = nullptr;
        layout_binding.binding = binding;
        return layout_binding;
    }
};

#endif
