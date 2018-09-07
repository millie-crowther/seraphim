#ifndef MESH_H
#define MESH_H

#include <vector>
#include "buffer.h" 
#include "vertex.h"
#include <string>
#include "texture.h"

class mesh_t {
private:
    // fields
    buffer_t * vertices;
    buffer_t * indices;
    texture_t * texture;
    int index_count;

public:
    // constructors and destructors
    mesh_t(
        VkCommandPool cmd_pool, VkQueue queue, const std::vector<vertex_t>& vertices, 
        const std::vector<uint32_t>& indices, texture_t * tx
    );
    ~mesh_t();

    // getters
    buffer_t * get_vertex_buffer();
    buffer_t * get_index_buffer();
    texture_t * get_texture();
    int get_index_count();

    // factory methods
    static mesh_t * load(std::string name, VkCommandPool pool, VkQueue queue, VkDevice device);
};

#endif
