#include "render/mesh.h"

#include <stdexcept>

mesh_t::mesh_t(
    VkCommandPool cmd_pool, VkQueue queue, const std::vector<vec_t<float, 2>> & vs, 
    const std::vector<uint32_t> & is
){
    VkDeviceSize v_size = sizeof(vec_t<float, 2>) * vs.size();
    vertices = std::make_shared<raw_buffer_t>(
        v_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    vertices->copy(cmd_pool, queue, (void *) vs.data(), v_size);

    VkDeviceSize i_size = sizeof(uint32_t) * is.size();
    indices = std::make_shared<raw_buffer_t>(
        i_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    indices->copy(cmd_pool, queue, (void *) is.data(), i_size);

    index_count = is.size();
}

buffer_t 
mesh_t::get_vertex_buffer() const {
    return vertices;    
}

buffer_t 
mesh_t::get_index_buffer() const {
    return indices;
}

int
mesh_t::get_index_count() const { 
    return index_count;
}
