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

    window->keyboard = std::make_unique<keyboard_t>(*window);
    mouse_create(&window->mouse, window);
}
