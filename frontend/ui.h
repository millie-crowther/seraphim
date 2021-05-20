#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "../common/maths.h"

typedef struct mouse_t {
    vec2 velocity;
    vec2 cursor_position;
    GLFWwindow *window;
} mouse_t;

typedef int keycode_t;

typedef struct keyboard_t {
    bool key_state[GLFW_KEY_LAST];
} keyboard_t;

typedef struct window_t {
    vec2u size;
    GLFWwindow *window;
    keyboard_t keyboard;
    mouse_t mouse;
} window_t;

void window_create(window_t * window, vec2u * size);
void window_destroy(window_t * window);
bool window_should_close(window_t *window);
void window_show(window_t * window);
void window_set_title(window_t *window, const char *title);

void mouse_create(mouse_t *mouse, window_t *window);
void mouse_update(mouse_t *mouse, double delta);

void keyboard_create(keyboard_t * keyboard, GLFWwindow * window);
bool keyboard_is_key_pressed(const keyboard_t *keyboard, keycode_t keycode);

#endif
