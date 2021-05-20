#ifndef MOUSE_H
#define MOUSE_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include "../common/maths.h"
#include <memory>

struct window_t;

struct mouse_t {
    vec2 velocity;
    vec2 cursor_position;
    struct window_t * window;
};

void mouse_create(mouse_t *mouse, struct window_t *window);
void mouse_update(mouse_t *mouse, double delta);

#endif
