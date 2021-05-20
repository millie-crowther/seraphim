#include "mouse.h"

#include "window.h"

void mouse_update(mouse_t *mouse, double delta) {
    vec2 cursor;
    glfwGetCursorPos(mouse->window->window, &cursor.x, &cursor.y);

    vec2_subtract(&mouse->velocity, &cursor, &mouse->cursor_position);
    vec2_divide_f(&mouse->velocity, &mouse->velocity, delta);
    glfwSetCursorPos(mouse->window->window, mouse->cursor_position.x, mouse->cursor_position.y);

}

void mouse_create(mouse_t *mouse, struct window_t *window) {
    mouse->window = window;
    mouse->cursor_position = {{(double) window->size.x, (double) window->size.y }};
    vec2_divide_f(&mouse->cursor_position, &mouse->cursor_position, 2.0);
    glfwSetCursorPos(window->window, mouse->cursor_position.x, mouse->cursor_position.y);

    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(window->window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    glfwSetInputMode(window->window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}
