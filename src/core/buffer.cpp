#include "core/buffer.h"

buffer_t::buffer_t(uint32_t binding, std::shared_ptr<device_t> device, uint64_t size, usage_t usage){
    this->device = device;
    this->size = size;
    this->binding = binding;
    this->usage = usage;

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkMemoryPropertyFlagBits memory_property;

    if (usage == device_local){
        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        memory_property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    } else if (usage == host_to_device){
        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        memory_property = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    } else if (usage == device_to_host){
        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        memory_property = static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
    } else if (usage == host_local){
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        memory_property = static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    if (vkCreateBuffer(device->get_device(), &buffer_info, nullptr, &buffer) != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to create buffer.");
    }

    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(device->get_device(), buffer, &mem_req);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = find_memory_type(device, mem_req.memoryTypeBits, memory_property);

    if (vkAllocateMemory(device->get_device(), &alloc_info, nullptr, &memory) != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to allocate buffer memory.");
    } 

    vkBindBufferMemory(device->get_device(), buffer, memory, 0); 

    desc_buffer_info = {};
    desc_buffer_info.buffer = buffer;
    desc_buffer_info.offset = 0;
    desc_buffer_info.range  = size;

    if (usage != host_local){
        staging_buffer = std::make_unique<buffer_t>(~0, device, size, host_local);
    }
}

buffer_t::~buffer_t(){
    vkDestroyBuffer(device->get_device(), buffer, nullptr);
    vkFreeMemory(device->get_device(), memory, nullptr);
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

VkBuffer
buffer_t::get_buffer() const {
    return buffer;
}

uint32_t 
buffer_t::find_memory_type(std::shared_ptr<device_t> device, uint32_t type_filter, VkMemoryPropertyFlags prop){
    VkPhysicalDeviceMemoryProperties mem_prop;
    vkGetPhysicalDeviceMemoryProperties(device->get_physical_device(), &mem_prop);

    for (uint32_t i = 0; i < mem_prop.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (mem_prop.memoryTypes[i].propertyFlags & prop) == prop) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}
