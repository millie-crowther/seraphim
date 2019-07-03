#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <memory>
#include <queue>

#include "render/texture.h"
#include "core/allocator.h"
#include "core/buffer.h"
#include "render/painter.h"
#include "sdf/sdf.h"

// forward declaration of brick
class brick_t;

class texture_manager_t {
private:
    // private fields
    std::unique_ptr<texture_t> colour_texture;
    std::unique_ptr<texture_t> geometry_texture;

    uint32_t claimed_patches;
    std::queue<uint32_t> patches;

    std::unique_ptr<buffer_t> staging_buffer;

    allocator_t allocator;

public:
    // constructors and destructors
    texture_manager_t(const allocator_t & allocator, const std::vector<VkDescriptorSet> & desc_sets);

    // mutators
    uint32_t request(u8vec4_t colour, u8vec4_t normal);
    void clear(uint32_t id);
};

#endif