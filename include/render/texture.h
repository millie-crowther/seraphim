#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <memory>

#include "vk_mem_alloc.h"

#include "core/device.h"
#include "maths/vec.h"

class texture_t {
private:
    VkImage image;
    VkImageView image_view;
    VkDeviceMemory memory;
    VkFormat format;
    VkImageLayout layout;
    VmaAllocation allocation;
    VkSampler sampler;
    VkDescriptorImageInfo image_info;
    uint32_t binding;

    VmaAllocator allocator;
    std::shared_ptr<device_t> device;

    // helper methods
    int find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties);
    
public:
    // constructors and destructors
    texture_t(
        uint32_t binding,
        VmaAllocator allocator, std::shared_ptr<device_t> device,
        u32vec2_t & size, VkImageUsageFlags usage, 
        VmaMemoryUsage vma_usage
    );
    ~texture_t();

    // getters
    VkImage get_image();
    VkImageView get_image_view();
    VkFormat get_format();
    VkImageLayout get_image_layout() const;
    VkSampler get_sampler() const;
    VkWriteDescriptorSet get_descriptor_write(VkDescriptorSet desc_set) const;

    // static methods
    void check_format_supported(
        VkPhysicalDevice physical_device, VkFormat format, 
        VkImageTiling tiling, VkFormatFeatureFlags features 
    );
    static VkImageView create_image_view(VkDevice device, VkImage image, VkFormat format);
};

#endif
