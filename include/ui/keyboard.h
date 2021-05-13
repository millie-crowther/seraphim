#ifndef KEYBOARD_H
#define KEYBOARD_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <map>
#include "mouse.h"

// forward declaration of window class
struct window_t;

struct keyboard_t {
    typedef int keycode_t;

    keyboard_t(const window_t &window);

    bool is_key_pressed(int key) const;

    void set_key_pressed(int key, bool state);

    std::map<keycode_t, bool> key_state;
};


#endif
