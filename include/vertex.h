#ifndef VERTEX_H
#define VERTEX_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <array>
#include "vec.h"

struct vertex_t {
    vec3_t position;
    vec3_t colour;

    vertex_t(vec3_t p, vec3_t c){
        position = p;
        colour = c;
    } 

    static VkVertexInputBindingDescription get_binding_description(){
        VkVertexInputBindingDescription binding_desc = {};
        binding_desc.binding = 0;
        binding_desc.stride = sizeof(vertex_t);
        binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding_desc;
    }

    static std::array<VkVertexInputAttributeDescription, 2> get_attr_descriptions(){
        std::array<VkVertexInputAttributeDescription, 2> attr_desc;

        attr_desc[0].binding = 0;
        attr_desc[0].location = 0;
        attr_desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attr_desc[0].offset = offsetof(vertex_t, position);

        attr_desc[1].binding = 0;
        attr_desc[1].location = 1;
        attr_desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attr_desc[1].offset = offsetof(vertex_t, colour);

        return attr_desc;
    }
};

#endif
