//
// Created by millie on 02/05/2021.
//

#include "core/buffer.h"

void buffer_create(buffer_t *self, uint32_t binding, device_t *device, uint64_t size,
                   bool is_device_local, size_t element_size) {
    self->is_device_local = is_device_local;
    self->device = device;
    self->element_size = element_size;
    self->size = element_size * size;
    self->binding = binding;

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = self->size;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkMemoryPropertyFlagBits memory_property;

    if (is_device_local) {
        buffer_info.usage =
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    } else {
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        memory_property = static_cast<VkMemoryPropertyFlagBits>(
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    if (vkCreateBuffer(device->device, &buffer_info, nullptr, &self->buffer) !=
        VK_SUCCESS) {
        throw std::runtime_error("Error: Failed to create buffer.");
    }

    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(device->device, self->buffer, &mem_req);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex =
        device_memory_type(device, mem_req.memoryTypeBits, memory_property);

    if (vkAllocateMemory(device->device, &alloc_info, nullptr, &self->memory) !=
        VK_SUCCESS) {
        throw std::runtime_error("Error: Failed to allocate buffer memory.");
    }

    if (vkBindBufferMemory(device->device, self->buffer, self->memory, 0) !=
        VK_SUCCESS) {
        throw std::runtime_error("Error: Failed to bind buffer memory.");
    }

    self->desc_buffer_info = {};
    self->desc_buffer_info.buffer = self->buffer;
    self->desc_buffer_info.offset = 0;
    self->desc_buffer_info.range = self->size;

    if (is_device_local) {
        self->staging_buffer = (buffer_t *)malloc(sizeof(*self->staging_buffer));
        buffer_create(self->staging_buffer, ~0, device, size, false, element_size);
    } else {
        self->staging_buffer = NULL;
    }
}

void buffer_destroy(buffer_t *self) {
    vkDestroyBuffer(self->device->device, self->buffer, nullptr);
    vkFreeMemory(self->device->device, self->memory, nullptr);

    if (self->staging_buffer != NULL) {
        buffer_destroy(self->staging_buffer);
        self->staging_buffer = NULL;
    }
}

size_t buffer_size(buffer_t *self) {
    return self->size / self->element_size;
}
