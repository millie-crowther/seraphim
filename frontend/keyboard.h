#ifndef KEYBOARD_H
#define KEYBOARD_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <map>
#include "mouse.h"

typedef int keycode_t;

typedef struct keyboard_t {
    bool key_state[GLFW_KEY_LAST];
} keyboard_t;

void keyboard_create(keyboard_t * keyboard, GLFWwindow * window);
bool keyboard_is_key_pressed(const keyboard_t *keyboard, keycode_t keycode);

#endif
