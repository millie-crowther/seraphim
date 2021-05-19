#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <memory>

#include "ui/keyboard.h"
#include "ui/mouse.h"
#include "mouse.h"
#include "keyboard.h"

struct window_t {
    vec2u size;
    GLFWwindow *window;
    keyboard_t keyboard;
    mouse_t mouse;
};

void window_create(window_t * window, vec2u * size);
void window_destroy(window_t * window);
bool window_should_close(window_t *window);
void window_show(window_t * window);
void window_set_title(window_t *window, const char *title);

#endif
