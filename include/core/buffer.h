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

    void *map(uint64_t offset, uint64_t map_size) {
        if (is_device_local) {
            return staging_buffer->map(offset, map_size);
        } else {
            void *memory_map;
            vkMapMemory(device->device, memory, element_size * offset,
                        element_size * map_size, 0, &memory_map);
            return memory_map;
        }
    }

    void unmap() {
        if (is_device_local) {
            staging_buffer->unmap();
        } else {
            vkUnmapMemory(device->device, memory);
        }
    }

    void write(const void *source, size_t number, uint64_t offset) {
        if (number == 0) {
            return;
        }

        if (element_size * (offset + number) > size + 1) {
            throw std::runtime_error("Error: Invalid buffer write.");
        }

        void *mem_map = map(offset, number);
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
        vkCmdCopyBuffer(command_buffer, staging_buffer->buffer, buffer,
                        updates.size(), updates.data());
        updates.clear();
    }

    void record_read(VkCommandBuffer command_buffer) const {
        VkBufferCopy region;
        region.srcOffset = 0;
        region.dstOffset = 0;
        region.size = size;
        vkCmdCopyBuffer(command_buffer, buffer, staging_buffer->buffer, 1, &region);
        vkCmdFillBuffer(command_buffer, buffer, 0, size, ~0);
    }

    VkWriteDescriptorSet
    get_write_descriptor_set(VkDescriptorSet descriptor_set) const {
        VkWriteDescriptorSet write_desc_set = {};
        write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc_set.pNext = NULL;
        write_desc_set.dstArrayElement = 0;
        write_desc_set.descriptorCount = 1;
        write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write_desc_set.pImageInfo = NULL;
        write_desc_set.pTexelBufferView = NULL;
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
        layout_binding.pImmutableSamplers = NULL;
        layout_binding.binding = binding;
        return layout_binding;
    }
};

void buffer_create(buffer_t *self, uint32_t binding, device_t *device, uint64_t size,
                   bool is_device_local, size_t element_size);

void buffer_destroy(buffer_t *self);

size_t buffer_size(buffer_t *self);

#endif
