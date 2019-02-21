#ifndef MESH_H
#define MESH_H

#include <vector>
#include "buffer.h" 
#include "vertex.h"
#include <string>
#include "texture.h"
#include <memory>

class mesh_t {
private:
    // fields
    buffer_t * vertices;
    buffer_t * indices;
    int index_count;

public:
    // constructors and destructors
    mesh_t(
        VkCommandPool cmd_pool, VkQueue queue, const std::vector<vertex_t> & vertices, 
        const std::vector<uint32_t> & indices
    );
    ~mesh_t();

    // getters
    buffer_t * get_vertex_buffer() const;
    buffer_t * get_index_buffer() const;
    int get_index_count() const;

    // factory methods
    // TODO: add a proxy to stop it always being loaded
    static std::shared_ptr<mesh_t> load(std::string name, VkCommandPool pool, VkQueue queue);
};

#endif
