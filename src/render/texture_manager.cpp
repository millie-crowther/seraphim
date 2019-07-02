#include "render/texture_manager.h"

#include "core/vk_utils.h"

#include "render/brick.h"

texture_manager_t::texture_manager_t(const allocator_t & allocator, uint16_t grid_size, const std::vector<VkDescriptorSet> & desc_sets){
    this->grid_size = grid_size;
    this->allocator = allocator;
    claimed_patches = 0;
    
    u32vec2_t image_size(grid_size * hyper::pi);

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VmaMemoryUsage vma_usage = VMA_MEMORY_USAGE_GPU_ONLY;
    

    colour_texture = std::make_unique<texture_t>(2, allocator, image_size, usage, vma_usage);
    geometry_texture = std::make_unique<texture_t>(3, allocator, image_size, usage, vma_usage);

    std::vector<VkWriteDescriptorSet> descriptor_write;

    for (auto desc_set : desc_sets){
        descriptor_write.push_back(colour_texture->get_descriptor_write(desc_set));
        descriptor_write.push_back(geometry_texture->get_descriptor_write(desc_set));
    }
    vkUpdateDescriptorSets(allocator.device, descriptor_write.size(), descriptor_write.data(), 0, nullptr);

    staging_buffer = std::make_unique<buffer_t>(
        allocator, hyper::pi * hyper::pi * sizeof(u8vec4_t),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY
    );

    colour_texture->transition_image_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    geometry_texture->transition_image_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
}

u16vec2_t 
texture_manager_t::request(
    const std::vector<u8vec4_t> & colour_patch,
    const std::vector<u8vec4_t> & geometry_patch
){
    u16vec2_t uv;

    if (claimed_patches < static_cast<uint32_t>(grid_size * grid_size)){
        claimed_patches++;
        uv = u16vec2_t(
            static_cast<uint16_t>(claimed_patches % grid_size), 
            static_cast<uint16_t>(claimed_patches / grid_size)
        );

    } else if (!patches.empty()){
        uv = patches.front();
        patches.pop();

    } else {
        throw std::runtime_error("No brick textures left!!");
    }

    staging_buffer->copy(colour_patch.data(), colour_patch.size() * sizeof(u8vec4_t), 0);

    staging_buffer->copy_to_image(
        colour_texture->get_image(), 
        uv.cast<uint32_t>() * hyper::pi,
        u32vec2_t(hyper::pi)
    );


    staging_buffer->copy(geometry_patch.data(), geometry_patch.size() * sizeof(u8vec4_t), 0);

    staging_buffer->copy_to_image(
        geometry_texture->get_image(), 
        uv.cast<uint32_t>() * hyper::pi,
        u32vec2_t(hyper::pi)
    );

    return uv;
}

void 
texture_manager_t::clear(u16vec2_t patch){
    patches.push(patch);
}