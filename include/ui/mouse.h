#ifndef MOUSE_H
#define MOUSE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "logic/scheduler.h"
#include "maths/vec.h"

class mouse_t {
private:
    vec2_t velocity;

public:
    mouse_t(const scheduler_t & scheduler, u32vec2_t window_size);
};

#endif