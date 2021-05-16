#include "render/texture.h"

#include <maths/maths.h>
#include <core/debug.h>
#include "core/buffer.h"

VkImageView texture_create_image_view(VkDevice device, VkImage image,
                                      VkFormat format) {
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
    if (vkCreateImageView(device, &view_info, NULL, &image_view) != VK_SUCCESS) {
        PANIC("Error: Failed to create image view.");
    }
    return image_view;
}

static void check_format_supported(VkPhysicalDevice physical_device,
                                       VkFormat candidate, VkImageTiling tiling,
                                       VkFormatFeatureFlags features) {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(physical_device, candidate, &properties);

    if ((tiling == VK_IMAGE_TILING_OPTIMAL || tiling == VK_IMAGE_TILING_LINEAR) &&
        (properties.optimalTilingFeatures & features) != features) {
        PANIC("Error: Unsupported image format.");
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

void texture_t::write(vec3i *p, uint32_t *x) {
    uint32_t offset = (index++ % buffer_size(&staging_buffer));
    buffer_write(&staging_buffer, x, 1, offset);

    VkBufferImageCopy region;
    region.bufferOffset = offset * staging_buffer.element_size;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {p->x, p->y, p->z};
    region.imageExtent = {2, 2, 2};

    updates.push_back(region);
}

void texture_t::record_write(VkCommandBuffer command_buffer) {
    if (updates.empty()){
        return;
    }

    vkCmdCopyBufferToImage(command_buffer, staging_buffer.buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, updates.size(),
                           updates.data());
    updates.clear();
}

void texture_create(texture_t *texture, uint32_t binding, device_t *device, vec3u *size, VkImageUsageFlags usage,
                    VkFormatFeatureFlagBits format_feature, VkDescriptorType descriptor_type) {
    texture->binding = binding;
    texture->device = device;
    texture->descriptor_type = descriptor_type;
    texture->extents = {size->x, size->y, size->z};

    texture->format = VK_FORMAT_R8G8B8A8_UNORM;
    texture->layout = VK_IMAGE_LAYOUT_UNDEFINED;

    // create image
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    image_create_info.extent = texture->extents;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.format = texture->format;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = texture->layout;
    image_create_info.usage = usage;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    check_format_supported(device->physical_device, texture->format, image_create_info.tiling,
                           format_feature);

    // allocate memory
    if (vkCreateImage(device->device, &image_create_info, NULL, &texture->image) !=
        VK_SUCCESS) {
        PANIC("Error: Failed to create image.");
    }
    VkMemoryRequirements mem_req;
    vkGetImageMemoryRequirements(device->device, texture->image, &mem_req);

    VkMemoryAllocateInfo mem_alloc_info = {};
    mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc_info.allocationSize = mem_req.size;
    mem_alloc_info.memoryTypeIndex = device_memory_type(
            device, mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device->device, &mem_alloc_info, NULL, &texture->memory) !=
        VK_SUCCESS) {
        PANIC("Error: Failed to allocate image memory.");
    }

    if (vkBindImageMemory(device->device, texture->image, texture->memory, 0) != VK_SUCCESS) {
        PANIC("Error: Failed to bind image.");
    }
    // create image view
    texture->image_view = texture_create_image_view(device->device, texture->image, texture->format);

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

    if (vkCreateSampler(device->device, &sampler_info, NULL, &texture->sampler) !=
        VK_SUCCESS) {
        PANIC("Error: Failed to create texture sampler.");
    }

    texture->image_info = {};
    texture->image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    texture->image_info.imageView = texture->image_view;
    texture->image_info.sampler = texture->sampler;

    buffer_create(&texture->staging_buffer, ~0, device, staging_buffer_size, false,
                  sizeof(uint32_t) * 8);
}

void texture_destroy(texture_t *texture) {
    vkDestroyImageView(texture->device->device, texture->image_view, NULL);
    vkDestroyImage(texture->device->device, texture->image, NULL);
    vkFreeMemory(texture->device->device, texture->memory, NULL);
    vkDestroySampler(texture->device->device, texture->sampler, NULL);
    buffer_destroy(&texture->staging_buffer);
}

void texture_descriptor_layout_binding(texture_t *texture, VkDescriptorSetLayoutBinding * layout_binding) {
    layout_binding->pImmutableSamplers = NULL;
    layout_binding->binding = texture->binding;
    layout_binding->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_binding->descriptorCount = 1;
    layout_binding->stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
}
