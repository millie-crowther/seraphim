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

    bool is_debug;
    VkInstance instance;
    GLFWwindow * window;

public:
    engine_t(bool is_debug);

    void run();
};

#endif
