#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>

#include "logic/revelator.h"
#include "maths/vec.h"
#include "ui/keyboard.h"
#include "ui/mouse.h"

class window_t {
private:
    // private fields
    u32vec2_t size;
    GLFWwindow * window;
    std::shared_ptr<keyboard_t> keyboard;

public:
    // constructors and destructors
    window_t(u32vec2_t size);
    ~window_t();

    // public fields
    revelator_t<u32vec2_t> on_resize;
    
    // modifiers
    void set_title(const std::string & title);

    // accessors
    GLFWwindow * get_window() const;
    u32vec2_t get_size() const;
    bool should_close() const;
    std::shared_ptr<keyboard_t> get_keyboard() const;
};

#endif