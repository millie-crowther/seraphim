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
    std::unique_ptr<keyboard_t> keyboard;
    mouse_t mouse;
    window_t(vec2u *size);
    ~window_t();
};

bool window_should_close(window_t *window);
void window_show(window_t * window);
void window_set_title(window_t *window, const char *title);

#endif
