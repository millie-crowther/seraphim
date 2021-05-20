#include "ui.h"

#include "../common/debug.h"


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


static void key_callback(GLFWwindow *glfw_window, int key, int scancode, int action,
                         int mods) {
    window_t *window =  (window_t *)glfwGetWindowUserPointer(glfw_window);
    window->keyboard.key_state[key] = action != GLFW_RELEASE;
}

bool keyboard_is_key_pressed(const keyboard_t *keyboard, keycode_t keycode) {
    return keyboard->key_state[keycode];
}

void keyboard_create(keyboard_t *keyboard, GLFWwindow *window) {
    glfwSetKeyCallback(window, key_callback);

    for (int i = 0; i < GLFW_KEY_LAST; i++){
        keyboard->key_state[i] = false;
    }
}


static void window_resize_callback(GLFWwindow *glfw_window, int width, int height) {
    //    void * data = glfwGetWindowUserPointer(glfw_window);
    //   window_t * window = reinterpret_cast<window_t *>(data);

    // TODO: handle resize of window
}

void window_set_title(window_t *window, const char *title) {
    glfwSetWindowTitle(window->window, title);
}

void window_show(window_t *window) {
    glfwShowWindow(window->window);
}

bool window_should_close(window_t *window) {
    return glfwWindowShouldClose(window->window) || keyboard_is_key_pressed(&window->keyboard, GLFW_KEY_ESCAPE);
}

void window_destroy(window_t *window) {
    if (window->window != NULL) {
        glfwDestroyWindow(window->window);
    }
}

void window_create(window_t *window, vec2u *size) {
    window->size = *size;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window->window = glfwCreateWindow(size->x, size->y, "Seraphim", NULL, NULL);

    if (window->window == NULL) {
        PANIC("Error: Failed to create main window.");
    }

    glfwSetWindowUserPointer(window->window, (void *) window);
    glfwSetWindowSizeCallback(window->window, window_resize_callback);

    keyboard_create(&window->keyboard, window->window);
    mouse_create(&window->mouse, window);
}
