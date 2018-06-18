#ifndef ENGINE_H
#define ENGINE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class engine_t {
private:
    void init();
    void update();
    void cleanup();

    bool should_quit(); 

    VkInstance instance;
    GLFWwindow * window;

public:
    void run();
};

#endif
