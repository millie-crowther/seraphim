#ifndef TEXTURE_H
#define TEXTURE_H

#include "render/image.h"
#include <string>
#include <memory>

class texture_t {
private:
    // private fields
    std::unique_ptr<image_t> image;
    int width;
    int height;
    VkSampler sampler;
    VkDevice device;

public:
    // constructors and destructors
    texture_t(VmaAllocator allocator, std::string filename, VkCommandPool pool, VkQueue queue);
    ~texture_t();

    // getters
    VkImageView get_image_view();
    VkSampler get_sampler();
};

#endif
