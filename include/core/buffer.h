#ifndef BUFFER_H
#define BUFFER_H

#include "core/command.h"
#include "core/device.h"
#include "array.h"

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
    array_t(VkBufferCopy) updates;
};

void buffer_create(buffer_t *self, uint32_t binding, device_t *device, uint64_t size,
                   bool is_device_local, size_t element_size);
void buffer_destroy(buffer_t *self);
size_t buffer_size(buffer_t *self);
void buffer_memory_barrier(buffer_t *self, VkBufferMemoryBarrier *barrier);
void *buffer_map(buffer_t *buffer, uint64_t offset, uint64_t map_size);
void buffer_unmap(buffer_t *buffer);
void buffer_write(buffer_t *buffer, const void *source, size_t number, uint64_t offset);
void buffer_record_write(buffer_t *buffer, VkCommandBuffer command_buffer);
void buffer_record_read(buffer_t *buffer, VkCommandBuffer command_buffer);
VkWriteDescriptorSet buffer_write_descriptor_set(buffer_t *buffer, VkDescriptorSet descriptor_set);
VkDescriptorSetLayoutBinding buffer_descriptor_set_layout_binding(buffer_t *buffer);

#endif
