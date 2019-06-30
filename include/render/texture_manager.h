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

    uint16_t grid_size;
    uint32_t claimed_patches;
    std::queue<u16vec2_t> patches;

    std::unique_ptr<buffer_t> staging_buffer;

    allocator_t allocator;

public:
    // constants
    static constexpr uint8_t brick_size = 8;

    // constructors and destructors
    texture_manager_t(const allocator_t & allocator, uint16_t grid_size, const std::vector<VkDescriptorSet> & desc_sets);

    // mutators
    u16vec2_t request(const std::array<u8vec4_t, brick_size * brick_size> & image);
    void clear(u16vec2_t patch);
};

#endif