#include "core/buffer.h"

buffer_t::buffer_t(VmaAllocator allocator, uint32_t binding, std::shared_ptr<device_t> device, uint64_t size, VmaMemoryUsage vma_usage){
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

buffer_t::~buffer_t(){
    vmaDestroyBuffer(allocator, buffer, allocation);
}

VkWriteDescriptorSet 
buffer_t::get_write_descriptor_set(VkDescriptorSet descriptor_set) const {
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

VkDescriptorSetLayoutBinding 
buffer_t::get_descriptor_set_layout_binding() const {
    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    layout_binding.pImmutableSamplers = nullptr;
    layout_binding.binding = binding;
    return layout_binding;
}