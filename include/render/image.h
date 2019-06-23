#ifndef IMAGE_H
#define IMAGE_H

#include "core/allocator.h"

#include <vector>
#include "vk_mem_alloc.h"
#include "maths/vec.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class image_t {
private:
    bool is_swapchain;
    VkImage image;
    VkImageView image_view;
    VkDeviceMemory memory;
    VkFormat format;
    VkImageLayout layout;
    VmaAllocation allocation;
    allocator_t allocator; // TODO: dont store a copy here

    // helper methods
    int find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties);
    void create_image_view(VkImageAspectFlags aspect_flags);

public:
    // constructors and destructors
    image_t(
        const allocator_t & allocator,
        u32vec2_t & size, VkFormat format, 
        VkImageTiling tiling, VkImageUsageFlags usage, 
        VmaMemoryUsage vma_usage, VkImageAspectFlags aspect_flags
    );
    image_t(
        const allocator_t & allocator,
        VkImage image, VkFormat format, VkImageAspectFlags aspect_flags
    );
    ~image_t();

    void transition_image_layout(VkImageLayout new_layout);

    // getters
    VkImage get_image();
    VkImageView get_image_view();
    VkFormat get_format();

    // static methods
    static VkFormat find_supported_format(
        VkPhysicalDevice physical_device, const std::vector<VkFormat> & candidates, 
        VkImageTiling tiling, VkFormatFeatureFlags features 
    );
    static VkFormat find_depth_format(VkPhysicalDevice physical_device);
};

#endif
