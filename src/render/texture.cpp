#include "render/texture.h"

#include "stb_image.h"
#include <stdexcept>
#include "core/blaspheme.h"

texture_t::texture_t(VmaAllocator allocator, std::string filename, VkCommandPool pool, VkQueue queue){
    int channels;
    
    // load data
    stbi_uc * pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (pixels == nullptr){
        throw std::runtime_error("Error: Failed to load texture \"" + filename + "\".");
    }

    u32vec2_t size = u32vec2_t(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    
    image = std::make_unique<image_t>(
        allocator, size, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_ASPECT_COLOR_BIT
    );

    VkDeviceSize buffer_size = size[0] * size[1] * 4;
    buffer_t texture_data(
        allocator, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );

    texture_data.copy(pool, queue, pixels, buffer_size, 0); 

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
    
    if (vkCreateSampler(blaspheme_t::get_device(), &sampler_info, nullptr, &sampler) != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to create texture sampler.");
    } 

}

texture_t::~texture_t(){
    vkDestroySampler(blaspheme_t::get_device(), sampler, nullptr);
}

VkImageView
texture_t::get_image_view(){
    return image->get_image_view();
}

VkSampler
texture_t::get_sampler(){
    return sampler;
}
