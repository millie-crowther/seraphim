#ifndef MOUSE_H
#define MOUSE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>

#include "maths/vec.h"

// forward declaration of window
class window_t;

class mouse_t {
private:
    vec2_t v;
    vec2_t c;

public:
    mouse_t(window_t & window);

    vec2_t get_velocity() const;

    void update(double delta, const window_t & window);
};

#endif