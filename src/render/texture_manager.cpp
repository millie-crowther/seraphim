#include "render/texture_manager.h"

#include "core/vk_utils.h"

#include "render/brick.h"

texture_manager_t::texture_manager_t(const allocator_t & allocator, uint16_t size, const std::vector<VkDescriptorSet> & desc_sets){
    this->size = size;
    this->allocator = allocator;
    claimed_bricks = 0;
    
    u32vec2_t image_size(512);

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

    // VkClearColorValue clear_colour;
    // clear_colour.float32[0] = 0.5f;
    // clear_colour.float32[1] = 1.0f;
    // clear_colour.float32[2] = 0.5f;
    // clear_colour.float32[3] = 1.0f;
    
    // VkImageSubresourceRange range;
    // range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    // range.baseMipLevel   = 0;
    // range.levelCount     = 1;
    // range.baseArrayLayer = 0;
    // range.layerCount     = 1;

    // auto cmd_buf = vk_utils::pre_commands(allocator.device, allocator.pool, allocator.queue);
    //     vkCmdClearColorImage(cmd_buf, image->get_image(), image->get_image_layout(), &clear_colour, 1, &range);
    // vk_utils::post_commands(allocator.device, allocator.pool, allocator.queue, cmd_buf);

    staging_buffer = std::make_unique<buffer_t>(
        allocator, brick_size * brick_size * 4,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY
    );

    image->transition_image_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    std::vector<u32vec4_t> blank(512 * 512, u32vec4_t(~0));

    staging_buffer->copy(blank.data(), blank.size() * sizeof(u32vec4_t), 0);
    staging_buffer->copy_to_image(
        image->get_image(), 
        0, 0,
        512, 512
    );

    // TODO:
    image->transition_image_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

texture_manager_t::~texture_manager_t(){
    vkDestroySampler(allocator.device, sampler, nullptr);
}


u16vec2_t 
texture_manager_t::request(const std::array<f32vec4_t, brick_size * brick_size> & brick){
    u16vec2_t result;

    if (claimed_bricks < static_cast<uint32_t>(size * size)){
        claimed_bricks++;
        result = u16vec2_t(
            static_cast<uint16_t>(claimed_bricks % size), 
            static_cast<uint16_t>(claimed_bricks / size)
        );

    } else if (!bricks.empty()){
        result = bricks.front();
        bricks.pop();

    } else {
        throw std::runtime_error("No brick textures left!!");
    }

    // staging_buffer->copy(brick.data(), brick.size() * sizeof(f32vec4_t), 0);
    // staging_buffer->copy_to_image(
    //     image->get_image(), 
    //     result[0] * brick_size, result[1] * brick_size,
    //     brick_size, brick_size
    // );

    return result;
}

void 
texture_manager_t::clear(u16vec2_t brick){
    // std::cout << "return: " << brick[0] << ", " << brick[1] << std::endl;
    bricks.push(brick);
}


void texture_manager_t::update_image(){
    // image->transition_image_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}