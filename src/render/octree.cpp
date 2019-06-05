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
    std::cout << "about to paint octree" << std::endl;
    paint(0, universal_aabb, renderables);
    
    for (auto node : structure){
        std::cout << node << ' ';
    }
    std::cout << std::endl;
    std::cout << is_leaf_flag << std::endl;

    buffer = std::make_unique<buffer_t>(
        allocator, structure.size() * sizeof(uint32_t),
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );

    // copy to buffer
    buffer->copy(pool, queue, structure.data(), structure.size() * sizeof(uint32_t));

    std::cout << "octree size: " << structure.size() << std::endl;

    // write to descriptor sets
    VkDescriptorBufferInfo desc_buffer_info = {};
    desc_buffer_info.buffer = buffer->get_buffer();
    desc_buffer_info.offset = 0;
    desc_buffer_info.range = structure.size() * sizeof(uint32_t);

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

void
octree_t::request(const vec3_t & x, const vec3_t & camera){
    if (!universal_aabb.contains(x)){
        return;
    }

    aabb_t aabb = universal_aabb;
    uint32_t i = lookup(x, 0, aabb);

    // node already has adequate data
    if (structure[i] != null_node){
        return;
    }

    // remove invisible renderables
    std::vector<std::weak_ptr<renderable_t>> renderables;
    for (auto & renderable_ptr : universal_renderables){
        if (auto renderable = renderable_ptr.lock()){
            if (renderable->is_visible()){
                renderables.push_back(renderable_ptr);
            }
        }
    }
    
    uint32_t start = structure.size();
    subdivide(i, x, camera, aabb, renderables);
    uint32_t end = structure.size();

    // perform copy to GPU
    
}

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

void 
octree_t::subdivide(
    uint32_t i,
    const vec3_t & x, const vec3_t & camera, 
    aabb_t & aabb,
    std::vector<std::weak_ptr<renderable_t>> & renderables
){
    std::remove_if(renderables.begin(), renderables.end(), [&aabb](const std::weak_ptr<renderable_t> & renderable_ptr){
        if (auto renderable = renderable_ptr.lock()){
            if (renderable->intersects(aabb)){
                return false;
            }
        }
        return true;
    });

    // check for homogenous volume
    bool is_homogenous = std::any_of(renderables.begin(), renderables.end(), [&aabb](const std::weak_ptr<renderable_t> & renderable_ptr){
        if (auto renderable = renderable_ptr.lock()){
            if (renderable->contains(aabb)){
                return true;
            }
        }
        return false;
    });

    if (renderables.empty() || is_homogenous){
        structure[i] = is_leaf_flag | is_homogenous_flag;

        if (is_homogenous){
            structure[i] |= 1;
        }

        return;
    }

    if (is_leaf(x, camera, aabb, renderables)){
        // TODO
        structure[i] = is_leaf_flag | is_homogenous_flag;
        return;
    }

    // create pointer to final element in structure
    structure[i] = structure.size();

    // create children
    structure.resize(structure.size() + 8, null_node);

    int octant = aabb.get_octant(x);
    aabb.refine(octant);

    // tail recurse
    subdivide(structure[i] + octant, x, camera, aabb, renderables); 
}

bool 
octree_t::is_leaf(
    const vec3_t & x, const vec3_t & camera, const aabb_t & aabb, 
    const std::vector<std::weak_ptr<renderable_t>> & renderables
) const {
    // TODO
    return aabb.get_size() <= 0.1;
}

void 
octree_t::paint(uint32_t i, aabb_t & aabb, const std::vector<std::weak_ptr<renderable_t>> & renderables){
    bool is_leaf = aabb.get_size() <= 0.2;

    for (auto renderable_ptr : renderables){
        if (auto renderable = renderable_ptr.lock()){
            bool is_empty = !renderable->intersects(aabb);
            if (is_empty || renderable->contains(aabb) || is_leaf){
                structure[i] = is_leaf_flag | is_homogenous_flag | uint32_t(!is_empty);
                return;
            }
        }
    }

    structure[i] = structure.size();
    for (int octant = 0; octant < 8; octant++){
        structure.push_back(null_node);
    }

    // TODO: only pass renderables that intersect child volume down to next stag
    for (int octant = 0; octant < 8; octant++){
        aabb_t new_aabb = aabb;
        new_aabb.refine(octant);
        paint(structure[i] + octant, new_aabb, renderables);
    }
}