#ifndef BUFFER_H
#define BUFFER_H

#include "core/command.h"
#include "core/device.h"

#include <cstring>
#include <memory>

struct buffer_t {
    bool is_device_local;
    device_t *device;
    VkBuffer buffer;
    VkDeviceMemory memory;
    uint64_t size;
    uint32_t binding;
    size_t element_size;
    VkDescriptorBufferInfo desc_buffer_info;

    buffer_t *staging_buffer;
    std::vector<VkBufferCopy> updates;

    // constructors and destructors
    buffer_t(uint32_t binding, device_t *device, uint64_t size, bool is_device_local, size_t element_size) {
        this->is_device_local = is_device_local;
        this->device = device;
        this->element_size = element_size;
        this->size = element_size * size;
        this->binding = binding;

        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = this->size;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkMemoryPropertyFlagBits memory_property;

        if (is_device_local) {
            buffer_info.usage =
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        } else {
            buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            memory_property =
                    static_cast < VkMemoryPropertyFlagBits >
                    (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }

        if (vkCreateBuffer
                    (device->device, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("Error: Failed to create buffer.");
        }

        VkMemoryRequirements mem_req;
        vkGetBufferMemoryRequirements(device->device, buffer, &mem_req);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_req.size;
        alloc_info.memoryTypeIndex =
                device_memory_type(device, mem_req.memoryTypeBits, memory_property);

        if (vkAllocateMemory
                    (device->device, &alloc_info, nullptr, &memory) != VK_SUCCESS) {
            throw std::runtime_error("Error: Failed to allocate buffer memory.");
        }

        if (vkBindBufferMemory(device->device, buffer, memory, 0) != VK_SUCCESS) {
            throw std::runtime_error("Error: Failed to bind buffer memory.");
        }

        desc_buffer_info = {};
        desc_buffer_info.buffer = buffer;
        desc_buffer_info.offset = 0;
        desc_buffer_info.range = this->size;

        if (is_device_local) {
            staging_buffer = new buffer_t(~0, device, size, false, element_size);
        } else {
            staging_buffer = NULL;
        }
    }

    ~buffer_t() {
        vkDestroyBuffer(device->device, buffer, nullptr);
        vkFreeMemory(device->device, memory, nullptr);

        if (staging_buffer != NULL) {
            delete staging_buffer;
        }
    }

    void * map(uint64_t offset, uint64_t size){
        if (is_device_local) {
            return staging_buffer->map(offset, size);
        } else {
            void *memory_map;
            vkMapMemory(device->device, memory,
                        element_size * offset, element_size * size, 0, &memory_map);
            return memory_map;
        }
    }

    void unmap(){
        if (is_device_local) {
            staging_buffer->unmap();
        } else {
            vkUnmapMemory(device->device, memory);
        }
    }

    void write(const void * source, size_t number, uint64_t offset) {
        if (element_size * (offset + number) > size + 1) {
            throw std::runtime_error("Error: Invalid buffer write.");
        }

        void * mem_map = map(offset, number);
        memcpy(mem_map, source, element_size * number);
        unmap();

        if (is_device_local) {
            VkBufferCopy buffer_copy = {};
            buffer_copy.srcOffset = element_size * offset;
            buffer_copy.dstOffset = element_size * offset;
            buffer_copy.size = element_size * number;
            updates.push_back(buffer_copy);
        }
    }

    void record_write(VkCommandBuffer command_buffer) {
        vkCmdCopyBuffer(command_buffer, staging_buffer->buffer,
                        buffer, updates.size(), updates.data());
        updates.clear();
    }

    void record_read(VkCommandBuffer command_buffer) const {
        VkBufferCopy region;
        region.srcOffset = 0;
        region.dstOffset = 0;
        region.size = size;
        vkCmdCopyBuffer(command_buffer, buffer,
                        staging_buffer->buffer, 1, &region);
        vkCmdFillBuffer(command_buffer, buffer, 0, size, ~0);
    }

    VkWriteDescriptorSet get_write_descriptor_set(VkDescriptorSet
                                                  descriptor_set) const {
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

    uint64_t get_size() const {
        return size / element_size;
    }

};

#endif
