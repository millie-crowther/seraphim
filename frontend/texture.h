#ifndef IMAGE_H
#define IMAGE_H

#include "buffer.h"
#include "command.h"
#include "device.h"
#include "../common/camera.h"

#include <vector>

static const uint32_t staging_buffer_size = 10000;

typedef struct texture_t {
    VkImage image;
    VkImageView image_view;
    VkDeviceMemory memory;
    VkFormat format;
    VkImageLayout layout;
    VkSampler sampler;
    VkDescriptorImageInfo image_info;
    VkDescriptorType descriptor_type;
    uint32_t binding;
    VkExtent3D extents;
    device_t *device;

    buffer_t staging_buffer;
    std::vector<VkBufferImageCopy> updates;
    uint32_t index;

    VkWriteDescriptorSet get_descriptor_write(VkDescriptorSet desc_set) const;

    void record_write(VkCommandBuffer command_buffer);
    void write(vec3i *p, uint32_t *x);
} texture_t;

void texture_create(texture_t * texture, uint32_t binding, device_t *device, vec3u *size,
VkImageUsageFlags usage, VkFormatFeatureFlagBits format_feature,
VkDescriptorType descriptor_type);

void texture_destroy(texture_t * texture);

VkImageView texture_create_image_view(VkDevice device, VkImage image,
                                      VkFormat format);

void texture_descriptor_layout_binding(texture_t *texture, VkDescriptorSetLayoutBinding * layout_binding);

#endif
