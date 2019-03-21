#ifndef IMAGE_H
#define IMAGE_H

#include <vector>

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

    // helper methods
    int find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties);
    void create_image_view(VkImageAspectFlags aspect_flags);

public:
    // constructors and destructors
    image_t(
        uint32_t width, uint32_t height, VkFormat format, 
        VkImageTiling tiling, VkImageUsageFlags usage, 
        VkMemoryPropertyFlags properties, VkImageAspectFlags aspect_flags
    );
    image_t(
        VkImage image, VkFormat format, VkImageAspectFlags aspect_flags
    );
    ~image_t();

    void transition_image_layout(VkCommandPool pool, VkQueue queue, VkImageLayout new_layout);

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
