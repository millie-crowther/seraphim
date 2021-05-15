#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <memory>

#include "maths/matrix.h"
#include "ui/keyboard.h"
#include "ui/mouse.h"
#include "mouse.h"
#include "keyboard.h"

struct window_t {
    srph::u32vec2_t size;
    GLFWwindow *window;
    std::unique_ptr<keyboard_t> keyboard;
    mouse_t mouse;
    window_t(srph::u32vec2_t size);
    ~window_t();
    void show();
    bool should_close() const;
};

void window_set_title(window_t *window, const char *title);

#endif
