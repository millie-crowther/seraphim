#include "render/vertex.h"

vertex_t::vertex_t(const vec3_t & p){
    position = p;
} 

VkVertexInputBindingDescription 
vertex_t::get_binding_description(){
    VkVertexInputBindingDescription binding_desc = {};
    binding_desc.binding = 0;
    binding_desc.stride = sizeof(vertex_t);
    binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding_desc;
}

std::array<VkVertexInputAttributeDescription, 1> 
vertex_t::get_attr_descriptions(){
    std::array<VkVertexInputAttributeDescription, 1> attr_desc;

    attr_desc[0].binding = 0;
    attr_desc[0].location = 0;
    attr_desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attr_desc[0].offset = offsetof(vertex_t, position);

    return attr_desc;
}