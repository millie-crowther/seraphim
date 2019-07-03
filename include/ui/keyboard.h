#ifndef KEYBOARD_H
#define KEYBOARD_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <map>

#include "logic/revelator.h"

class keyboard_t {
public:
    typedef int keycode_t;

    keyboard_t();
    ~keyboard_t();

    revelator_t<keycode_t> on_key_press;
    revelator_t<keycode_t> on_key_release;

    bool is_key_pressed(int key) const;

private:
    std::map<keycode_t, bool> key_state;
    uuid_t key_press_uuid;
    uuid_t key_release_uuid;
};

#endif 