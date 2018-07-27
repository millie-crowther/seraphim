#ifndef MESH_H
#define MESH_H

#include <vector>
#include "buffer.h" 
#include "vertex.h"
#include "sdf.h"

class mesh_t {
private:
    // fields
    buffer_t * vertices;
    buffer_t * indices;

public:
    // constructors and destructors
    mesh_t(
        VkCommandPool cmd_pool, VkQueue queue, const std::vector<vertex_t>& vertices, 
        const std::vector<unsigned int>& indices
    );
    ~mesh_t();

    // getters
    buffer_t * get_vertex_buffer();
    buffer_t * get_index_buffer();
};

#endif
