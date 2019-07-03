#include "ui/keyboard.h"

#include <iostream>

void
keyboard_t::key_event(int key, int action, int mods){
    key_state[key] = action == GLFW_PRESS || action == GLFW_REPEAT;
}

bool
keyboard_t::is_key_pressed(int key) const {
    key_state.find(key);
    return key_state.find(key) != key_state.end() && key_state.at(key);
}