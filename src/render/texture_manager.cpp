#include "render/texture_manager.h"

texture_manager_t::texture_manager_t(const allocator_t & allocator, uint16_t size){
    this->size = size;
    device = allocator.device;
    claimed_bricks = 0;
    
    u32vec2_t image_size(size, size);

    image = std::make_unique<image_t>(
        allocator, image_size, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_ASPECT_COLOR_BIT
    );

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

texture_manager_t::~texture_manager_t(){
    vkDestroySampler(device, sampler, nullptr);
}


u16vec2_t 
texture_manager_t::request(){
    if (claimed_bricks < size * size){
        claimed_bricks++;
        return u16vec2_t(claimed_bricks % size, claimed_bricks / size);

    } else if (!bricks.empty()){
        u16vec2_t result = bricks.front();
        bricks.pop();
        return result;

    } else {
        throw std::runtime_error("No brick textures left!!");
    }
}

void 
texture_manager_t::clear(u16vec2_t brick){
    // std::cout << "return: " << brick[0] << ", " << brick[1] << std::endl;
    bricks.push(brick);
}