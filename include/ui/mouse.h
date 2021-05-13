#ifndef MOUSE_H
#define MOUSE_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include "maths/matrix.h"
#include <memory>

// forward declaration of window
struct window_t;

struct mouse_t {
    srph::vec2_t v;
    srph::vec2_t c;

    mouse_t(window_t &window);

    srph::vec2_t get_velocity() const;

    void update(double delta, const window_t &window);
};


#endif
