#include "render/texture_manager.h"

#include "core/vk_utils.h"

#include "render/brick.h"

constexpr uint8_t texture_manager_t::brick_size;

texture_manager_t::texture_manager_t(const allocator_t & allocator, uint16_t size, const std::vector<VkDescriptorSet> & desc_sets){
    this->size = size;
    this->allocator = allocator;
    claimed_bricks = 0;
    
    u32vec2_t image_size(size * brick_size);

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
    
    if (vkCreateSampler(allocator.device, &sampler_info, nullptr, &sampler) != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to create texture sampler.");
    } 

    VkDescriptorImageInfo image_info = {};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = image->get_image_view();
    image_info.sampler = sampler;

    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstBinding = 2;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pImageInfo = &image_info;

    for (auto desc_set : desc_sets){
        descriptor_write.dstSet = desc_set;
        vkUpdateDescriptorSets(allocator.device, 1, &descriptor_write, 0, nullptr);
    }

    staging_buffer = std::make_unique<buffer_t>(
        allocator, brick_size * brick_size * sizeof(uint32_t),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY
    );

    image->transition_image_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    std::vector<uint32_t> blank(2048 * 2048, 0xFFFF00FF);

    // staging_buffer->copy(blank.data(), blank.size() * sizeof(uint32_t), 0);
    // staging_buffer->copy_to_image(
    //     image->get_image(), 
    //     u32vec2_t(0),
    //     u32vec2_t(2048)
    // );

    // TODO:
    // image->transition_image_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


    // std::vector<uint32_t> blank(brick_size * brick_size, 0xFFFF00FF);
    // staging_buffer->copy(blank.data(), blank.size() * sizeof(uint32_t), 0);

    // for (uint32_t x = 0; x < 256; x++){
    //     for (uint32_t y = 0; y < 256; y++){
    //         u32vec2_t uv(x, y);
    //         staging_buffer->copy_to_image(
    //             image->get_image(), 
    //             uv * brick_size,
    //             u32vec2_t(brick_size)
    //         );
    //     }
    // }
}

texture_manager_t::~texture_manager_t(){
    vkDestroySampler(allocator.device, sampler, nullptr);
}


u16vec2_t 
texture_manager_t::request(const std::array<colour_t, brick_size * brick_size> & brick){
    u16vec2_t uv;

    if (claimed_bricks < static_cast<uint32_t>(size * size)){
        claimed_bricks++;
        uv = u16vec2_t(
            static_cast<uint16_t>(claimed_bricks % size), 
            static_cast<uint16_t>(claimed_bricks / size)
        );

    } else if (!bricks.empty()){
        uv = bricks.front();
        bricks.pop();

    } else {
        throw std::runtime_error("No brick textures left!!");
    }

    // staging_buffer->copy(brick.data(), brick.size() * sizeof(uint32_t), 0);
    // staging_buffer->copy_to_image(
    //     image->get_image(), 
    //     uv.cast<uint32_t>(),
    //     u32vec2_t(brick_size)
    // );

    // std::cout << uv[0] << ", " << uv[1] << std::endl;

    std::vector<uint32_t> blank(brick_size * brick_size, 0xFFFF00FF);

    staging_buffer->copy(blank.data(), blank.size() * sizeof(uint32_t), 0);
    staging_buffer->copy_to_image(
        image->get_image(), 
        uv.cast<uint32_t>() * brick_size,
        u32vec2_t(brick_size)
    );

    return uv;
}

void 
texture_manager_t::clear(u16vec2_t brick){
    bricks.push(brick);
}