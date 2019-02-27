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

// std::shared_ptr<mesh_t>
// mesh_t::load(std::string name, VkCommandPool pool, VkQueue queue){
//     const std::string texture_name = "../resources/mesh/" + name + "/" + name + ".jpg";
//     texture_t * t = new texture_t(texture_name, pool, queue);
            
//     const std::string mesh_name = "../resources/mesh/" + name + "/" + name + ".obj";

//     tinyobj::attrib_t attr;
//     std::vector<tinyobj::shape_t> shapes;
//     std::vector<tinyobj::material_t> materials;
//     std::string err;
//     std::string warning;

//     if (!tinyobj::LoadObj(&attr, &shapes, &materials, &warning, &err, mesh_name.c_str())){
//         throw std::runtime_error("Error: Couldn't load mesh: " + err);
//     }

//     std::vector<vertex_t> vs;
//     std::vector<uint32_t> is;

//     for (const auto& shape : shapes){
//         for (const auto& index : shape.mesh.indices){
//             // flip axes to account for different system
//             vec3_t pos({
//                 attr.vertices[3 * index.vertex_index + 0],   
//                 attr.vertices[3 * index.vertex_index + 2],    
//                 -attr.vertices[3 * index.vertex_index + 1] 
//             });

//             // change tex coords for different standard
//             vec2_t tex_coord({
//                 attr.texcoords[2 * index.texcoord_index + 0],
//                 1.0f - attr.texcoords[2 * index.texcoord_index + 1]
//             });

//             vs.push_back(vertex_t(pos, vec3_t(1), tex_coord));
//             is.push_back(is.size());
//         }
//     }

//     return std::make_shared<mesh_t>(pool, queue, vs, is, t);
// }

int
mesh_t::get_index_count() const { 
    return index_count;
}
