#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <memory>
#include <queue>

#include "render/image.h"

class texture_manager_t {
private:
    // constants
    static constexpr uint16_t brick_size = 8;

    // private fields
    std::unique_ptr<image_t> image;

    uint16_t size;
    uint32_t unclaimed_bricks;
    std::queue<u16vec2_t> bricks;

    VkSampler sampler;
    VkDevice device;

public:
    // constructors and destructors
    texture_manager_t(VmaAllocator allocator, VkDevice device, VkCommandPool pool, VkQueue queue, uint16_t size);
    ~texture_manager_t();

    // mutators
    u16vec2_t request();
    void clear(u16vec2_t brick);
};

#endif