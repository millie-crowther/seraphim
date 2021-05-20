#include "keyboard.h"

#include "window.h"

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
