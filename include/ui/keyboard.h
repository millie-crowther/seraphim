#ifndef KEYBOARD_H
#define KEYBOARD_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <map>

namespace srph {
// forward declaration of window class
struct window_t;

class keyboard_t {
  public:
    typedef int keycode_t;

    keyboard_t(const window_t &window);

    bool is_key_pressed(int key) const;

    void set_key_pressed(int key, bool state);

  private:
    std::map<keycode_t, bool> key_state;
};
} // namespace srph
#endif
