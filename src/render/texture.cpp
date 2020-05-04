#include "render/texture.h"

#include <stdexcept>

#include "core/blaspheme.h"

texture_t::texture_t(
    VmaAllocator allocator,
    uint32_t binding, std::shared_ptr<device_t> device,
    u32vec3_t size, VkImageUsageFlags usage, VkMemoryPropertyFlagBits memory_property,
    VkFormatFeatureFlagBits format_feature, VkDescriptorType descriptor_type
){    
    this->binding = binding;
    this->device = device;
    this->descriptor_type = descriptor_type;

    format = VK_FORMAT_R8G8B8A8_UNORM;
    layout = VK_IMAGE_LAYOUT_UNDEFINED;

    // create image
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    image_create_info.extent.width = size[0];
    image_create_info.extent.height = size[1];
    image_create_info.extent.depth = size[2];
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.format = format;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = layout;
    image_create_info.usage = usage;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    check_format_supported(
        device->get_physical_device(), format,
        image_create_info.tiling, format_feature 
    );

    // allocate memory 
    if (vkCreateImage(device->get_device(), &image_create_info, nullptr, &image) != VK_SUCCESS){
	    throw std::runtime_error("Error: Failed to create image.");
    }
    VkMemoryRequirements mem_req;
    vkGetImageMemoryRequirements(device->get_device(), image, &mem_req);

    VkMemoryAllocateInfo mem_alloc_info = {};
    mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc_info.allocationSize = mem_req.size;
    mem_alloc_info.memoryTypeIndex = find_memory_type(mem_req.memoryTypeBits, memory_property);

    if (vkAllocateMemory(device->get_device(), &mem_alloc_info, nullptr, &memory) != VK_SUCCESS){
	    throw std::runtime_error("Error: Failed to allocate image memory.");
    }

    if (vkBindImageMemory(device->get_device(), image, memory, 0) != VK_SUCCESS){
	    throw std::runtime_error("Error: Failed to create image.");
    }

    // create image view
    image_view = create_image_view(device->get_device(), image, format);

    // create sampler
    VkSamplerCreateInfo sampler_info = {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;
    
    if (vkCreateSampler(device->get_device(), &sampler_info, nullptr, &sampler) != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to create texture sampler.");
    } 

    image_info = {};
    image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_info.imageView = image_view;
    image_info.sampler = sampler;

    staging_buffer = std::make_unique<buffer_t>(
        allocator, ~0, device, sizeof(uint32_t) * 8, VMA_MEMORY_USAGE_CPU_ONLY
    );
}

VkFormat
texture_t::get_format(){
    return format;
}

VkSampler 
texture_t::get_sampler() const {
    return sampler;
}

VkImageLayout
texture_t::get_image_layout() const {
    return layout;
}

VkImageView
texture_t::create_image_view(VkDevice device, VkImage image, VkFormat format){
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkImageView image_view;
    if (vkCreateImageView(device, &view_info, nullptr, &image_view) != VK_SUCCESS){
	    throw std::runtime_error("Error: Failed to create image view.");
    }
    return image_view;
}

texture_t::~texture_t(){
    vkDestroyImageView(device->get_device(), image_view, nullptr);
    vkDestroyImage(device->get_device(), image, nullptr);
    vkFreeMemory(device->get_device(), memory, nullptr);
    vkDestroySampler(device->get_device(), sampler, nullptr);
}

int
texture_t::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties){
    VkPhysicalDeviceMemoryProperties memory_prop;
    vkGetPhysicalDeviceMemoryProperties(device->get_physical_device(), &memory_prop);

    for (uint32_t i = 0; i < memory_prop.memoryTypeCount; i++){
        if (
            (type_filter & (1 << i)) &&
            (memory_prop.memoryTypes[i].propertyFlags & properties) == properties
        ){
            return i;
        }
    }

    throw std::runtime_error("Error: Failed to find appropriate memory type.");
}

VkImageView
texture_t::get_image_view(){
    return image_view;
}

void
texture_t::check_format_supported(
    VkPhysicalDevice physical_device, VkFormat candidate, 
    VkImageTiling tiling, VkFormatFeatureFlags features
){
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(physical_device, candidate, &properties);

    if (
        (tiling == VK_IMAGE_TILING_OPTIMAL || tiling == VK_IMAGE_TILING_LINEAR) &&
        (properties.optimalTilingFeatures & features) != features
    ){
        throw std::runtime_error("Error: Unsupported image format.");
    }  
}

VkWriteDescriptorSet 
texture_t::get_descriptor_write(VkDescriptorSet desc_set) const {
    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstBinding = binding;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = descriptor_type;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pImageInfo = &image_info;
    descriptor_write.dstSet = desc_set;

    return descriptor_write;
}

void 
texture_t::transition_image_layout(const command_pool_t & command_pool, bool is_pre_transfer){
    VkImageLayout new_layout = is_pre_transfer ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    auto command_buffer = command_pool.one_time_buffer([&](auto command_buffer){
        VkImageMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        barrier.oldLayout     = is_pre_transfer ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout     = new_layout;
        barrier.srcAccessMask = is_pre_transfer ? 0 : VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = is_pre_transfer ? VK_ACCESS_TRANSFER_WRITE_BIT : VK_ACCESS_SHADER_READ_BIT;

        VkPipelineStageFlags source_stage = is_pre_transfer ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_TRANSFER_BIT;
        VkPipelineStageFlags destination_stage = is_pre_transfer ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

        vkCmdPipelineBarrier(
            command_buffer, source_stage, destination_stage,
            0, 0, nullptr, 0, nullptr, 1, &barrier
        );
    });

    command_buffer->submit(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE);

    layout = new_layout;
}

VkImage 
texture_t::get_image() const {
    return image;
}

void 
texture_t::write(const command_pool_t & command_pool, const std::array<uint32_t, 8> & x){
    staging_buffer->write(x, 0);

    transition_image_layout(command_pool, true);

    // auto command_buffer = command_pool.one_time_buffer([&](auto command_buffer){
    //     VkBufferImageCopy region;
    //     region.bufferOffset = 0;
    //     region.bufferRowLength = 0;
    //     region.bufferImageHeight = 0;
    //     region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //     region.imageSubresource.mipLevel = 0;
    //     region.imageSubresource.baseArrayLayer = 0;
    //     region.imageSubresource.layerCount = 1;
    //     region.imageOffset = { 0, 0, 0 };
    //     region.imageExtent = { 2, 2, 2 };

    //     vkCmdCopyBufferToImage(command_buffer, staging_buffer->get_buffer(), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    // });

    // command_buffer->submit(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE);

    transition_image_layout(command_pool, false);
}