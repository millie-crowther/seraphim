#include "texture.h"

#include "stb_image.h"
#include <stdexcept>
#include "buffer.h"

texture_t::texture_t(std::string filename, VkCommandPool pool, VkQueue queue, VkDevice device){
    this->device = device;

    int channels;
    
    // load data
    stbi_uc * pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (pixels == nullptr){
        throw std::runtime_error("Error: Failed to load texture \"" + filename + "\".");
    }

    VkDeviceSize size = width * height * 4;
    image = new image_t(
        width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT
    );

    buffer_t texture_data(
        size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    texture_data.copy(pool, queue, pixels, size); // pool, queue

    stbi_image_free(pixels);

    texture_data.copy_to_image(pool, queue, image->get_image(), width, height);

    // create sampler
    VkSamplerCreateInfo sampler_info = {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = 16;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;
    
    if (vkCreateSampler(device, &sampler_info, nullptr, &sampler) != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to create texture sampler.");
    } 

}

texture_t::~texture_t(){
    vkDestroySampler(device, sampler, nullptr);

    delete image;
}

VkImageView
texture_t::get_image_view(){
    return image->get_image_view();
}

VkSampler
texture_t::get_sampler(){
    return sampler;
}
