#include "render/texture_manager.h"

#include "core/vk_utils.h"

#include "render/brick.h"

texture_manager_t::texture_manager_t(const allocator_t & allocator, const std::vector<VkDescriptorSet> & desc_sets){
    this->allocator = allocator;
    claimed_patches = 1;
    
    u32vec2_t image_size(hyper::tau);

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VmaMemoryUsage vma_usage = VMA_MEMORY_USAGE_GPU_ONLY;
    

    colour_texture = std::make_unique<texture_t>(3, allocator, image_size, usage, vma_usage);
    geometry_texture = std::make_unique<texture_t>(4, allocator, image_size, usage, vma_usage);

    std::vector<VkWriteDescriptorSet> descriptor_write;

    for (auto desc_set : desc_sets){
        descriptor_write.push_back(colour_texture->get_descriptor_write(desc_set));
        descriptor_write.push_back(geometry_texture->get_descriptor_write(desc_set));
    }
    vkUpdateDescriptorSets(allocator.device, descriptor_write.size(), descriptor_write.data(), 0, nullptr);

    staging_buffer = std::make_unique<buffer_t>(
        allocator, sizeof(u8vec4_t),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY
    );

    colour_texture->transition_image_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    geometry_texture->transition_image_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
}

uint32_t 
texture_manager_t::request(u8vec4_t colour, u8vec4_t normal){
    uint32_t id;

    if (claimed_patches < hyper::tau * hyper::tau){
        id = claimed_patches;
        claimed_patches++;

    } else if (!patches.empty()){
        id = patches.front();
        patches.pop();

    } else {
        throw std::runtime_error("No brick textures left!!");
    }

    u32vec2_t uv = u32vec2_t(id % hyper::tau, id / hyper::tau);

    staging_buffer->copy(&colour, sizeof(u8vec4_t), 0);
    staging_buffer->copy_to_image(
        colour_texture->get_image(), 
        uv, u32vec2_t(1)
    );

    staging_buffer->copy(&normal, sizeof(u8vec4_t), 0);
    staging_buffer->copy_to_image(
        geometry_texture->get_image(), 
        uv, u32vec2_t(1)
    );

    return id;
}

void 
texture_manager_t::clear(uint32_t id){
    patches.push(id);
}