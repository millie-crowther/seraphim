#ifndef IMAGE_H
#define IMAGE_H

#include "core/allocator.h"

#include <vector>
#include "vk_mem_alloc.h"
#include "maths/vec.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
    allocator_t allocator; // TODO: dont store a copy here

    // helper methods
    int find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties);
    
public:
    // constructors and destructors
    texture_t(
        uint32_t binding,
        const allocator_t & allocator,
        u32vec2_t & size, VkImageUsageFlags usage, 
        VmaMemoryUsage vma_usage
    );
    ~texture_t();

    void transition_image_layout(VkImageLayout new_layout);

    // getters
    VkImage get_image();
    VkImageView get_image_view();
    VkFormat get_format();
    VkImageLayout get_image_layout() const;
    VkSampler get_sampler() const;
    VkWriteDescriptorSet get_descriptor_write(VkDescriptorSet desc_set) const;

    // static methods
    static VkFormat find_supported_format(
        VkPhysicalDevice physical_device, const std::vector<VkFormat> & candidates, 
        VkImageTiling tiling, VkFormatFeatureFlags features 
    );
    static VkFormat find_depth_format(VkPhysicalDevice physical_device);
    static VkImageView create_image_view(VkDevice device, VkImage image, VkFormat format);
};

#endif
