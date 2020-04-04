#include "ui/window.h"

#include <exception>

static void 
window_resize_callback(GLFWwindow * glfw_window, int width, int height){
    void * data = glfwGetWindowUserPointer(glfw_window);
    window_t * window = reinterpret_cast<window_t *>(data);
    window->on_resize.announce(u32vec2_t((uint32_t) width, (uint32_t) height));
}

window_t::window_t(u32vec2_t size, std::weak_ptr<scheduler_t> scheduler){
    this->size = size;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(size[0], size[1], "BLASPHEME", nullptr, nullptr);

    if (window == nullptr){
        throw std::runtime_error("Error: Failed to create main window.");
    }

    glfwSetWindowUserPointer(window, static_cast<void *>(this));
    glfwSetWindowSizeCallback(window, window_resize_callback); 

    keyboard = std::make_unique<keyboard_t>(*this);
    mouse = std::make_unique<mouse_t>(*this, scheduler);
}

window_t::~window_t(){
    if (window != nullptr){
        glfwDestroyWindow(window);
    }
}

u32vec2_t 
window_t::get_size() const {
    return size;
}

GLFWwindow * 
window_t::get_window() const {
    return window;
}

bool 
window_t::should_close() const {
    return glfwWindowShouldClose(window) || keyboard->is_key_pressed(GLFW_KEY_ESCAPE);
}

void 
window_t::set_title(const std::string & title){
    glfwSetWindowTitle(window, title.c_str());
}

const keyboard_t &
window_t::get_keyboard() const {
    return *keyboard;
}

const mouse_t &
window_t::get_mouse() const {
    return *mouse;
}
