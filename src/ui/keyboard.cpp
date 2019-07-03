#include "ui/keyboard.h"

#include "ui/window.h"

static void 
key_callback(GLFWwindow * glfw_window, int key, int scancode, int action, int mods){
    void * data = glfwGetWindowUserPointer(glfw_window);
    window_t * window = reinterpret_cast<window_t *>(data);
    
    if (action == GLFW_PRESS){
        window->get_keyboard().on_key_press.announce(key);
    } else if (action == GLFW_RELEASE){
        window->get_keyboard().on_key_release.announce(key);
    }   
}

keyboard_t::keyboard_t(const window_t & window){
    key_press_follower = on_key_press.follow([&](keycode_t key){
        key_state[key] = true;
    });

    key_release_follower = on_key_release.follow([&](keycode_t key){
        key_state[key] = false;
    });

    glfwSetKeyCallback(window.get_window(), key_callback);
}

bool
keyboard_t::is_key_pressed(int key) const {
    key_state.find(key);
    return key_state.find(key) != key_state.end() && key_state.at(key);
}