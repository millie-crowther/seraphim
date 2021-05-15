#include "ui/window.h"

#include <exception>
#include <core/debug.h>

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
    return glfwWindowShouldClose(window->window) ||
           window->keyboard->is_key_pressed(GLFW_KEY_ESCAPE);}

window_t::window_t(vec2u *size) {
    this->size = *size;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window = glfwCreateWindow(size->x, size->y, "Seraphim", NULL, NULL);

    if (window == NULL) {
        PANIC("Error: Failed to create main window.");
    }

    glfwSetWindowUserPointer(window, static_cast<void *>(this));
    glfwSetWindowSizeCallback(window, window_resize_callback);

    keyboard = std::make_unique<keyboard_t>(*this);
    mouse_create(&mouse, this);
}

window_t::~window_t() {
    if (window != NULL) {
        glfwDestroyWindow(window);
    }
}
