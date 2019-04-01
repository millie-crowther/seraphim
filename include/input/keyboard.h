#ifndef KEYBOARD_H
#define KEYBOARD_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <map>

class keyboard_t {
private:
    std::map<int, bool> key_state;

public:
    void key_event(int key, int action, int mods);

    bool is_key_pressed(int key);
};

#endif 