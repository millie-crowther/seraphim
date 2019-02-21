#ifndef VERTEX_H
#define VERTEX_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <array>
#include "maths/vec.h"

class vertex_t {
private:
    // private fields
    vec3_t position;

public:
    // constructors and destructors
    vertex_t(const vec3_t & p);

    // static methods
    static VkVertexInputBindingDescription get_binding_description();
    static std::array<VkVertexInputAttributeDescription, 1> get_attr_descriptions();
};

#endif
