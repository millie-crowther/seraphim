#ifndef IMAGE_H
#define IMAGE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class image_t {
private:
    VkImage image;
    VkImageView image_view;
    VkDeviceMemory memory;
    VkFormat format;
    VkImageLayout layout;

public:
    // constructors and destructors
    image_t(
        VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
	VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspect_flags
    );
    image_t(VkImage image);
    ~image_t();

    void transition_image_layout(VkImageLayout new_layout);

};

#endif
