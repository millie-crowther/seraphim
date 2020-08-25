#include "ui/mouse.h"

#include "ui/window.h"

mouse_t::mouse_t(window_t & window){
    GLFWwindow * w = window.get_window();
    c = window.get_size() / 2.0;
    glfwSetCursorPos(w, c[0], c[1]);
  
    if (glfwRawMouseMotionSupported()){
        glfwSetInputMode(w, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    
    glfwSetInputMode(window.get_window(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

vec2_t
mouse_t::get_velocity() const {
    return v;
}

void
mouse_t::update(double delta, const window_t & window){
    vec2_t cursor;
    glfwGetCursorPos(window.get_window(), &cursor[0], &cursor[1]);

    v = (cursor - c) / delta;
    glfwSetCursorPos(window.get_window(), c[0], c[1]);
}
