#include "render/texture.h"

#include <stdexcept>

#include "core/blaspheme.h"
#include "core/vk_utils.h"

texture_t::texture_t(
    uint32_t binding,
    VmaAllocator allocator, std::shared_ptr<device_t> device,
    u32vec2_t & size, VkImageUsageFlags usage, 
    VmaMemoryUsage vma_usage
){    
    this->binding = binding;
    format = VK_FORMAT_R8G8B8A8_UNORM;
    
    this->allocator = allocator;
    this->device = device;
    
    layout = VK_IMAGE_LAYOUT_UNDEFINED;

    // create image
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = size[0];
    image_create_info.extent.height = size[1];
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.format = format;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = layout;
    image_create_info.usage = usage;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // VK_FORMAT_R8G8B8A8_UNORM

    check_format_supported(
        device->get_physical_device(),
        format,
        image_create_info.tiling, 
        VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT 
    );

    // VmaAllocationCreateInfo alloc_create_info = {};
    // alloc_create_info.usage = vma_usage; 

    // VmaAllocation allocation;
    // VmaAllocationInfo alloc_info;

    // TODO: Use VMA to do the allocation. not sure why below approach doesn't work.
    //       screen goes black if you try this way
    //  
    //
    // VkResult result = vmaCreateImage(
    //     allocator, &image_create_info, &alloc_create_info,
    //     &image, &allocation, &alloc_info
    // );

    // if (result != VK_SUCCESS){
	//     throw std::runtime_error("Error: Failed to create image.");
    // }

    // memory = alloc_info.deviceMemory;


    // // allocate memory 
    VkResult result = vkCreateImage(device->get_device(), &image_create_info, nullptr, &image);
    if (result != VK_SUCCESS){
	    throw std::runtime_error("Error: Failed to create image.");
    }
    VkMemoryRequirements mem_req;
    vkGetImageMemoryRequirements(device->get_device(), image, &mem_req);

    VkMemoryAllocateInfo mem_alloc_info = {};
    mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc_info.allocationSize = mem_req.size;
    auto properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    mem_alloc_info.memoryTypeIndex = find_memory_type(mem_req.memoryTypeBits, properties);

    result = vkAllocateMemory(device->get_device(), &mem_alloc_info, nullptr, &memory);
    if (result != VK_SUCCESS){
	    throw std::runtime_error("Error: Failed to allocate image memory.");
    }

    result = vkBindImageMemory(device->get_device(), image, memory, 0);

    if (result != VK_SUCCESS){
	    throw std::runtime_error("Error: Failed to create image.");
    }

    // create image view
    image_view = create_image_view(device->get_device(), image, format);

    // create sampler
    VkSamplerCreateInfo sampler_info = {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
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
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkImageView image_view;
    VkResult result = vkCreateImageView(device, &view_info, nullptr, &image_view);
    if (result != VK_SUCCESS){
	    throw std::runtime_error("Error: Failed to create image view.");
    }

    return image_view;
}

texture_t::~texture_t(){
    vkDestroyImageView(device->get_device(), image_view, nullptr);
    
    vkDestroyImage(device->get_device(), image, nullptr);
    vkFreeMemory(device->get_device(), memory, nullptr);
    // vmaDestroyImage(allocator, image, allocation);
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

    return -1;
}

VkImage
texture_t::get_image(){
    return image;
}

void
texture_t::transition_image_layout(VkImageLayout new_layout, VkCommandPool pool, VkQueue queue){
    auto cmd = vk_utils::pre_commands(device->get_device(), pool, queue);
        VkImageLayout old_layout = layout;      
        
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags src_stage;
        VkPipelineStageFlags dst_stage;

        if (
            old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
            new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        ){
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (
            old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
            new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        ){
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (
            old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
            new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        ){
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
                                  | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            throw std::runtime_error("Error: Unsupported layout transition.");
        }

        if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL){
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            // if has stencil component
            if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT){
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        vkCmdPipelineBarrier(
            cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier
        );
    vk_utils::post_commands(device->get_device(), pool, queue, cmd);

    layout = new_layout;
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
        tiling == VK_IMAGE_TILING_LINEAR &&
        (properties.linearTilingFeatures & features) != features
    ){
        throw std::runtime_error("Error: Unsupported image format.");
    }

    if (
        tiling == VK_IMAGE_TILING_OPTIMAL &&
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
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pImageInfo = &image_info;
    descriptor_write.dstSet = desc_set;

    return descriptor_write;
}