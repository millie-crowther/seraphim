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
//    self->updates = std::vector<VkBufferCopy>();

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

void buffer_memory_barrier(buffer_t *self, VkBufferMemoryBarrier *buffer_barrier) {
    *buffer_barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = NULL,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
        .srcQueueFamilyIndex = self->device->compute_family,
        .dstQueueFamilyIndex = self->device->compute_family,
        .buffer = self->buffer,
        .offset = 0,
        .size = VK_WHOLE_SIZE,
    };
}

void *buffer_map(buffer_t *buffer, uint64_t offset, uint64_t map_size) {
    if (buffer->is_device_local) {
        return buffer_map(buffer->staging_buffer, offset, map_size);
    } else {
        void *memory_map;
        vkMapMemory(buffer->device->device, buffer->memory, buffer->element_size * offset,
                    buffer->element_size * map_size, 0, &memory_map);
        return memory_map;
    }
}

void buffer_unmap(buffer_t *buffer) {
    if (buffer->is_device_local) {
        buffer_unmap(buffer->staging_buffer);
    } else {
        vkUnmapMemory(buffer->device->device, buffer->memory);
    }
}

void buffer_write(buffer_t *buffer, const void *source, size_t number, uint64_t offset) {
    if (number == 0) {
        return;
    }

    if (buffer->element_size * (offset + number) > buffer->size + 1) {
        throw std::runtime_error("Error: Invalid buffer write.");
    }

    void *mem_map = buffer_map(buffer, offset, number);
    memcpy(mem_map, source, buffer->element_size * number);
    buffer_unmap(buffer);

    if (buffer->is_device_local) {
        VkBufferCopy buffer_copy = {};
        buffer_copy.srcOffset = buffer->element_size * offset;
        buffer_copy.dstOffset = buffer->element_size * offset;
        buffer_copy.size = buffer->element_size * number;
        buffer->updates.push_back(buffer_copy);
    }
}

void buffer_record_write(buffer_t *buffer, VkCommandBuffer command_buffer) {
    if (buffer->updates.empty()){
        return;
    }

    vkCmdCopyBuffer(command_buffer, buffer->staging_buffer->buffer, buffer->buffer,
                    buffer->updates.size(), buffer->updates.data());
    buffer->updates.clear();
}

void buffer_record_read(buffer_t *buffer, VkCommandBuffer command_buffer) {
    VkBufferCopy region;
    region.srcOffset = 0;
    region.dstOffset = 0;
    region.size = buffer->size;
    vkCmdCopyBuffer(command_buffer, buffer->buffer, buffer->staging_buffer->buffer, 1, &region);
    vkCmdFillBuffer(command_buffer, buffer->buffer, 0, buffer->size, 0);
}

VkWriteDescriptorSet buffer_write_descriptor_set(buffer_t *buffer, VkDescriptorSet descriptor_set) {
    VkWriteDescriptorSet write_desc_set = {};
    write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_desc_set.pNext = NULL;
    write_desc_set.dstArrayElement = 0;
    write_desc_set.descriptorCount = 1;
    write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write_desc_set.pImageInfo = NULL;
    write_desc_set.pTexelBufferView = NULL;
    write_desc_set.dstSet = descriptor_set;
    write_desc_set.dstBinding = buffer->binding;
    write_desc_set.pBufferInfo = &buffer->desc_buffer_info;
    return write_desc_set;
}

VkDescriptorSetLayoutBinding buffer_descriptor_set_layout_binding(buffer_t *buffer) {
    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    layout_binding.pImmutableSamplers = NULL;
    layout_binding.binding = buffer->binding;
    return layout_binding;
}