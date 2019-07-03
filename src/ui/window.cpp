#include "ui/window.h"

static void 
window_resize_callback(GLFWwindow * glfw_window, int width, int height){
    void * data = glfwGetWindowUserPointer(glfw_window);
    window_t * window = reinterpret_cast<window_t *>(data);
    window->on_resize.announce(u32vec2_t((uint32_t) width, (uint32_t) height));
}

static void 
key_callback(GLFWwindow * glfw_window, int key, int scancode, int action, int mods){
    // void * data = glfwGetWindowUserPointer(glfw_window);
    // window_t * window = reinterpret_cast<window_t *>(data);
    // window->keyboard_event(key, action, mods);
}

window_t::window_t(u32vec2_t size){
    this->size = size;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);


    window = glfwCreateWindow(size[0], size[1], "BLASPHEME", nullptr, nullptr);

    glfwSetWindowUserPointer(window, static_cast<void *>(this));
    glfwSetWindowSizeCallback(window, window_resize_callback);   
    glfwSetKeyCallback(window, key_callback);

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
    return glfwWindowShouldClose(window);
}

void 
window_t::set_title(const std::string & title){
    glfwSetWindowTitle(window, title.c_str());
}