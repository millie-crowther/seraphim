#ifndef IMAGE_H
#define IMAGE_H

#include "core/buffer.h"
#include "core/command.h"
#include "core/device.h"
#include "maths/matrix.h"

#include <vector>

struct texture_t {
    static constexpr uint32_t staging_buffer_size = 10000;

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

    // constructors and destructors
    texture_t(uint32_t binding, device_t *device, srph::u32vec3_t size,
              VkImageUsageFlags usage, VkFormatFeatureFlagBits format_feature,
              VkDescriptorType descriptor_type);
    ~texture_t();

    VkWriteDescriptorSet get_descriptor_write(VkDescriptorSet desc_set) const;

    void record_write(VkCommandBuffer command_buffer);
    void write(srph::u32vec3_t p, uint32_t *x);
    VkDescriptorSetLayoutBinding get_descriptor_layout_binding() const;

    // static methods
    static void check_format_supported(VkPhysicalDevice physical_device,
                                       VkFormat format, VkImageTiling tiling,
                                       VkFormatFeatureFlags features);
    static VkImageView create_image_view(VkDevice device, VkImage image,
                                         VkFormat format);
};

#endif
