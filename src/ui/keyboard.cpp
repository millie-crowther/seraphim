#include "ui/keyboard.h"

keyboard_t::keyboard_t(){
    key_press_uuid = on_key_press.follow([&](keycode_t key){
        key_state[key] = true;
    });

    key_release_uuid = on_key_release.follow([&](keycode_t key){
        key_state[key] = false;
    });
}

keyboard_t::~keyboard_t(){
    on_key_press.renounce(key_press_uuid);
    on_key_release.renounce(key_release_uuid);
}

bool
keyboard_t::is_key_pressed(int key) const {
    key_state.find(key);
    return key_state.find(key) != key_state.end() && key_state.at(key);
}