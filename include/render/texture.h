#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <memory>

#include "vk_mem_alloc.h"

#include "core/buffer.h"
#include "core/command.h"
#include "core/device.h"
#include "maths/vec.h"

class texture_t {
private:
    VkImage image;
    VkImageView image_view;
    VkDeviceMemory memory;
    VkFormat format;
    VkImageLayout layout;
    VkSampler sampler;
    VkDescriptorImageInfo image_info;
    VkDescriptorType descriptor_type;
    uint32_t binding;

    std::shared_ptr<device_t> device;

    std::unique_ptr<buffer_t> staging_buffer;

    // helper methods
    int find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties);

public:
    // constructors and destructors
    texture_t(
        VmaAllocator allocator,
        uint32_t binding, std::shared_ptr<device_t> device,
        u32vec3_t size, VkImageUsageFlags usage, VkMemoryPropertyFlagBits memory_property,
        VkFormatFeatureFlagBits format_feature, VkDescriptorType descriptor_type
    );
    ~texture_t();

    // getters
    VkImage get_image() const;
    VkImageView get_image_view();
    VkFormat get_format();
    VkImageLayout get_image_layout() const;
    VkSampler get_sampler() const;
    VkWriteDescriptorSet get_descriptor_write(VkDescriptorSet desc_set) const; 

    void write(const command_pool_t & command_pool, uint32_t i, const std::array<uint32_t, 8> & x);

    // static methods
    static void check_format_supported(
        VkPhysicalDevice physical_device, VkFormat format, 
        VkImageTiling tiling, VkFormatFeatureFlags features 
    );
    static VkImageView create_image_view(VkDevice device, VkImage image, VkFormat format);
};

#endif
