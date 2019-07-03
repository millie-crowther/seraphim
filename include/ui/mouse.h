#ifndef MOUSE_H
#define MOUSE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>

#include "logic/scheduler.h"
#include "maths/vec.h"

// forward declaration of window
class window_t;

class mouse_t {
private:
    vec2_t v;
    vec2_t c;
    std::weak_ptr<scheduler_t> scheduler;

    interval_revelator_t::follower_ptr_t frame_start_follower;
    revelator_t<u32vec2_t>::follower_ptr_t window_resize_follower;

public:
    mouse_t(window_t & window, std::weak_ptr<scheduler_t> scheduler);

    vec2_t get_velocity() const;
};

#endif