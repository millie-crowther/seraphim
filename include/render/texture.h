#ifndef TEXTURE_H
#define TEXTURE_H

#include "image.h"
#include <string>

class texture_t {
private:
    // private fields
    image_t * image;
    int width;
    int height;
    VkSampler sampler;
    VkDevice device;

public:
    // constructors and destructors
    texture_t(std::string filename, VkCommandPool pool, VkQueue queue);
    ~texture_t();

    // getters
    VkImageView get_image_view();
    VkSampler get_sampler();
};

#endif
