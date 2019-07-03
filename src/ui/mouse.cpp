#include "ui/mouse.h"

#include "ui/window.h"

mouse_t::mouse_t(window_t & window, std::weak_ptr<scheduler_t> scheduler_ptr){
    GLFWwindow * w = window.get_window();
    c = window.get_size().cast<double>() / 2;
    glfwSetCursorPos(w, c[0], c[1]);
  
    if (glfwRawMouseMotionSupported()){
        glfwSetInputMode(w, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    
    glfwSetInputMode(window.get_window(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    if (auto scheduler = scheduler_ptr.lock()){
        frame_start_follower = scheduler->on_frame_start.follow([w, this](double delta){
            vec2_t cursor;
            glfwGetCursorPos(w, &cursor[0], &cursor[1]);

            v = (cursor - c) / delta;
            glfwSetCursorPos(w, c[0], c[1]);
        });
    }

    window_resize_follower = window.on_resize.follow([&](u32vec2_t size){
        c = size.cast<double>() / 2;
    });
}

vec2_t
mouse_t::get_velocity() const {
    return v;
}