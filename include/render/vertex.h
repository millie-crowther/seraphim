#ifndef VERTEX_H
#define VERTEX_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <array>
#include "vec.h"

class vertex_t {
private:
    // private fields
    vec3_t position;
    vec3_t colour;
    vec2_t tex_coord;

public:
    // constructors and destructors
    vertex_t(vec3_t p, vec3_t c, vec2_t tc);

    // overloaded operators
    bool operator==(const vertex_t& v);

    // static methods
    static VkVertexInputBindingDescription get_binding_description();
    static std::array<VkVertexInputAttributeDescription, 3> get_attr_descriptions();
};

namespace std {
    template<> struct hash<vertex_t>{
        size_t operator()(vertex_t const& vertex) const {
            //TODO
            return 0;
        } 
    };
}

#endif
