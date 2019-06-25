#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <memory>
#include <queue>

#include "render/image.h"
#include "core/allocator.h"
#include "core/buffer.h"
#include "render/painter.h"
#include "sdf/sdf.h"

// forward declaration of brick
class brick_t;

class texture_manager_t {
private:
    // private fields
    std::unique_ptr<image_t> image;

    uint16_t size;
    uint32_t claimed_bricks;
    std::queue<u16vec2_t> bricks;

    std::unique_ptr<buffer_t> staging_buffer;

    VkSampler sampler;
    allocator_t allocator;


public:
    // constants
    static constexpr uint8_t brick_size = 8;

    // constructors and destructors
    texture_manager_t(const allocator_t & allocator, uint16_t size, const std::vector<VkDescriptorSet> & desc_sets);
    ~texture_manager_t();

    // mutators
    u16vec2_t request(const std::array<f32vec4_t, brick_size * brick_size> & image);
    void clear(u16vec2_t brick);
    void update_image();
};

#endif