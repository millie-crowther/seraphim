#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <memory>

#include "vk_mem_alloc.h"

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

    // helper methods
    int find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties);
    
public:
    // constructors and destructors
    texture_t(
        uint32_t binding, std::shared_ptr<device_t> device,
        u32vec3_t size, VkImageUsageFlags usage, VkMemoryPropertyFlagBits memory_property,
        VkFormatFeatureFlagBits format_feature, VkDescriptorType descriptor_type
    );
    ~texture_t();

    // getters
    VkImageView get_image_view();
    VkFormat get_format();
    VkImageLayout get_image_layout() const;
    VkSampler get_sampler() const;
    VkWriteDescriptorSet get_descriptor_write(VkDescriptorSet desc_set) const; 

    void transition_image_layout(const command_pool_t & command_pool, VkImageLayout oldLayout, VkImageLayout newLayout);

    // static methods
    static void check_format_supported(
        VkPhysicalDevice physical_device, VkFormat format, 
        VkImageTiling tiling, VkFormatFeatureFlags features 
    );
    static VkImageView create_image_view(VkDevice device, VkImage image, VkFormat format);
};

#endif
