#include "ui/keyboard.h"

#include "ui/window.h"

static void key_callback(GLFWwindow *glfw_window, int key, int scancode, int action,
                         int mods) {
    void *data = glfwGetWindowUserPointer(glfw_window);
    window_t *window = reinterpret_cast<window_t *>(data);

    (*window->keyboard).set_key_pressed(key, action != GLFW_RELEASE);
}

keyboard_t::keyboard_t(const window_t &window) {
    glfwSetKeyCallback(window.window, key_callback);
}

bool keyboard_t::is_key_pressed(int key) const {
    key_state.find(key);
    return key_state.find(key) != key_state.end() && key_state.at(key);
}

void keyboard_t::set_key_pressed(int key, bool state) { key_state[key] = state; }