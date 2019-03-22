#ifndef MESH_H
#define MESH_H

#include <vector>
#include "buffer.h" 
#include "maths/vec.h"

class mesh_t {
private:
    // fields
    buffer_t vertices;
    buffer_t indices;
    int index_count;

public:
    // constructors and destructors
    mesh_t(
        VkCommandPool cmd_pool, VkQueue queue, const std::vector<vec_t<float, 2>> & vertices, 
        const std::vector<uint32_t> & indices
    );

    // getters
    buffer_t get_vertex_buffer() const;
    buffer_t get_index_buffer() const;
    int get_index_count() const;
};

#endif
