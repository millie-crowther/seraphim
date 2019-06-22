#include "render/texture_manager.h"

texture_manager_t::texture_manager_t(const allocator_t & allocator, uint16_t size){
    this->size = size;
    device = allocator.device;
    unclaimed_bricks = static_cast<uint32_t>(size) * static_cast<uint32_t>(size);
    
    u32vec2_t image_size(size, size);

    image = std::make_unique<image_t>(
        allocator.vma_allocator, image_size, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
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
    // u16vec2_t result;


    // if (unclaimed_bricks > 0){
    //     // TODO

    //     unclaimed_bricks--;

    // } else if (!bricks.empty()){
    //     result = bricks.front();
    //     bricks.pop();
 
    // } else {
    //     // TODO
    // }

    // return result;

    std::cout << "request " << std::endl;

    return u16vec2_t(0);
}

void 
texture_manager_t::clear(u16vec2_t brick){
    bricks.push(brick);
}