#include "render/octree.h"

#include <iostream>
#include "core/blaspheme.h"

constexpr uint32_t octree_t::null_node;

octree_t::octree_t(
    VmaAllocator allocator, VkCommandPool pool, VkQueue queue, double render_distance, 
    const std::vector<std::weak_ptr<renderable_t>> & renderables, 
    const std::vector<VkDescriptorSet> & desc_sets
){

    universal_aabb = aabb_t(vec3_t(-render_distance), render_distance * 2);
    structure.push_back(null_node); 
    paint(0, universal_aabb, renderables);

    uint32_t size = sizeof(uint32_t) * max_structure_size + sizeof(f32vec4_t) * max_geometry_size;
    buffer = std::make_unique<buffer_t>(
        allocator, size,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );

    // copy to buffer
    buffer->copy(pool, queue, structure.data(), sizeof(uint32_t) * structure.size(), 0);
    buffer->copy(pool, queue, geometry.data(),  sizeof(f32vec4_t) * geometry.size(), sizeof(uint32_t) * max_structure_size);

    int leaf_nodes = 0;
    for (auto node : structure){
        if (node & is_leaf_flag){
            leaf_nodes++;
        }
    }

    // int redundant_nodes = 0;
    // for (int i = 1; i < structure.size(); i += 8){

    // }

    std::cout << "octree size: " << structure.size() << std::endl;
    std::cout << "leaf nodes "  << leaf_nodes << std::endl;

    // write to descriptor sets
    VkDescriptorBufferInfo desc_buffer_info = {};
    desc_buffer_info.buffer = buffer->get_buffer();
    desc_buffer_info.offset = 0;
    desc_buffer_info.range  = sizeof(uint32_t) * max_structure_size + sizeof(f32vec4_t) * max_geometry_size; 

    VkWriteDescriptorSet write_desc_set = {};
    write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_desc_set.pNext = nullptr;
    write_desc_set.dstBinding = 1;
    write_desc_set.dstArrayElement = 0;
    write_desc_set.descriptorCount = 1;
    write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write_desc_set.pImageInfo = nullptr;
    write_desc_set.pBufferInfo = &desc_buffer_info;
    write_desc_set.pTexelBufferView = nullptr;

    std::vector<VkWriteDescriptorSet> write_desc_sets(desc_sets.size());
    for (int i = 0; i < write_desc_sets.size(); i++){
        write_desc_sets[i] = write_desc_set;
        write_desc_sets[i].dstSet = desc_sets[i];
    }

    vkUpdateDescriptorSets(blaspheme_t::get_device(), write_desc_sets.size(), write_desc_sets.data(), 0, nullptr);
}

// void
// octree_t::request(const vec3_t & x, const vec3_t & camera){
//     if (!universal_aabb.contains(x)){
//         return;
//     }

//     aabb_t aabb = universal_aabb;
//     uint32_t i = lookup(x, 0, aabb);

//     // node already has adequate data
//     if (structure[i] != null_node){
//         return;
//     }

//     // remove invisible renderables
//     std::vector<std::weak_ptr<renderable_t>> renderables;
//     for (auto & renderable_ptr : universal_renderables){
//         if (auto renderable = renderable_ptr.lock()){
//             if (renderable->is_visible()){
//                 renderables.push_back(renderable_ptr);
//             }
//         }
//     }
    
//     uint32_t start = structure.size();
//     subdivide(i, x, camera, aabb, renderables);
//     uint32_t end = structure.size();

//     // perform copy to GPU
    
// }


uint32_t
octree_t::lookup(const vec3_t & x, uint32_t i, aabb_t & aabb) const {
    // base cases
    if (structure[i] == null_node || (structure[i] & is_leaf_flag)) {
        return i;
    } 
    
    // refine octant
    int octant = aabb.get_octant(x);
    aabb.refine(octant);
   
    // tail recurse
    return lookup(x, structure[i] + octant, aabb);
}

vec4_t 
octree_t::get_plane(
    const std::vector<std::weak_ptr<renderable_t>> & renderables,
    const vec3_t & x
) const {
    std::vector<vec4_t> planes;
    for (auto renderable_ptr : renderables){
        if (auto renderable = renderable_ptr.lock()){
            planes.push_back(renderable->plane(x));
        }
    }
    
    return std::accumulate(planes.begin(), planes.end(), vec4_t()) / std::max(planes.size(), 1);
}

void 
octree_t::paint(uint32_t i, aabb_t & aabb, const std::vector<std::weak_ptr<renderable_t>> & renderables){
    bool is_leaf = aabb.get_size() <= 0.25;

    std::vector<std::weak_ptr<renderable_t>> new_renderables;

    for (auto renderable_ptr : renderables){
        if (auto renderable = renderable_ptr.lock()){
            if (renderable->contains(aabb)){
                structure[i] = is_leaf_flag;
                return;
            }

            if (renderable->intersects(aabb)){
                new_renderables.push_back(renderable_ptr);
            }
        }
    }

    if (new_renderables.empty()){
        structure[i] = is_leaf_flag;
        return;

    } else if (is_leaf){
        structure[i] = is_leaf_flag | geometry.size();

        // TODO: do union of all intersecting renderables
        geometry.push_back(new_renderables[0].lock()->plane(aabb.get_centre()).cast<float>()); 
        return;
    } 

    structure[i] = structure.size();
    for (uint8_t octant = 0; octant < 8; octant++){
        structure.push_back(null_node);
    }

    for (uint8_t octant = 0; octant < 8; octant++){
        aabb_t new_aabb = aabb;
        new_aabb.refine(octant);
        paint(structure[i] + octant, new_aabb, renderables);
    }
}