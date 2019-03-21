#include "render/mesh.h"

#include "tiny_obj_loader.h"
#include <stdexcept>

mesh_t::mesh_t(
    VkCommandPool cmd_pool, VkQueue queue, const std::vector<vertex_t> & vs, 
    const std::vector<uint32_t> & is
){
    VkDeviceSize v_size = sizeof(vertex_t) * vs.size();
    vertices = new buffer_t(
        v_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    vertices->copy(cmd_pool, queue, (void *) vs.data(), v_size);

    VkDeviceSize i_size = sizeof(uint32_t) * is.size();
    indices = new buffer_t(
        i_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    indices->copy(cmd_pool, queue, (void *) is.data(), i_size);

    index_count = is.size();
}

mesh_t::~mesh_t(){
    delete vertices;
    delete indices;
}

buffer_t *
mesh_t::get_vertex_buffer() const {
     return vertices;
}

buffer_t * 
mesh_t::get_index_buffer() const {
    return indices;
}

int
mesh_t::get_index_count() const { 
    return index_count;
}
