#include "render/octree.h"

#include <iostream>
#include "core/blaspheme.h"
#include <set>
#include "sdf/compose.h"
#include "sdf/mutate.h"
#include "maths/mat.h"

constexpr uint32_t octree_t::null_node;

octree_t::octree_t(
    const allocator_t & allocator, 
    const std::vector<std::weak_ptr<sdf3_t>> & sdfs, 
    const std::vector<VkDescriptorSet> & desc_sets,
    std::weak_ptr<camera_t> camera
){
    structure.reserve(max_structure_size);
    
    // TODO: remove this! its only here because the zero index is 
    //       regarded as an empty volume in the shader 
    structure.emplace_back(null_node); 

    universal_aabb = vec4_t(-hyper::rho);
    universal_aabb[3] = hyper::rho * 2;

    texture_manager = std::make_shared<texture_manager_t>(allocator, desc_sets);
    
    if (auto c = camera.lock()){
        paint(0, universal_aabb, sdfs, c);
    }

    std::cout << "octree size: " << structure.size() << std::endl;
    std::cout << "brickset size: " << brickset.size() << std::endl;

    uint32_t size = sizeof(uint32_t) * max_structure_size + sizeof(f32vec4_t) * max_brickset_size;
    buffer = std::make_unique<buffer_t>(
        allocator, size,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );

    // copy to buffer
    buffer->copy(structure.data(), sizeof(uint32_t) * structure.size(), 0);
    buffer->copy(device_brickset.data(),  sizeof(f32vec4_t) * brickset.size(), sizeof(uint32_t) * max_structure_size);

    // write to descriptor sets
    VkDescriptorBufferInfo desc_buffer_info = {};
    desc_buffer_info.buffer = buffer->get_buffer();
    desc_buffer_info.offset = 0;
    desc_buffer_info.range  = sizeof(uint32_t) * max_structure_size + sizeof(f32vec4_t) * max_brickset_size; 

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
    for (uint32_t i = 0; i < write_desc_sets.size(); i++){
        write_desc_sets[i] = write_desc_set;
        write_desc_sets[i].dstSet = desc_sets[i];
    }

    vkUpdateDescriptorSets(allocator.device, write_desc_sets.size(), write_desc_sets.data(), 0, nullptr);
}

uint32_t 
octree_t::create_brick(const vec4_t & aabb, const sdf3_t & sdf){
    brickset.emplace(aabb, texture_manager, sdf, &device_brickset[brickset.size()]);
    return brickset.size() - 1;
}

std::tuple<bool, bool> 
octree_t::intersects_contains(const vec4_t & aabb, std::shared_ptr<sdf3_t> sdf) const {
    double upper_radius = vec3_t(aabb[3] / 2).norm();
    vec3_t c = vec3_t(aabb[0], aabb[1], aabb[2]) + vec3_t(aabb[3] / 2.0f);
    double p = sdf->phi(c);

    // containment check upper bound
    if (p <= -upper_radius){
        return std::make_tuple(false, true);
    }

    // intersection check lower bound
    if (std::abs(p) <= aabb[3] / 2){
        return std::make_tuple(true, false);
    }

    // intersection check upper bound
    if (p >= upper_radius){
        return std::make_tuple(false, false);
    }

    // containment check precise
    double d = (sdf->normal(c) * p).chebyshev_norm();
    if (p < 0 && d > aabb[3] / 2){
        return std::make_tuple(false, true);
    }

    // intersection check precise
    if (d <= aabb[3] / 2){
        return std::make_tuple(true, false);
    }

    return std::make_tuple(false, false);
}

void 
octree_t::paint(
    uint32_t i, const vec4_t & aabb, 
    const std::vector<std::weak_ptr<sdf3_t>> & sdfs,
    std::shared_ptr<camera_t> camera
){
    std::vector<std::shared_ptr<sdf3_t>> new_sdfs;

    for (auto sdf_ptr : sdfs){
        if (auto sdf = sdf_ptr.lock()){
            auto intersection = intersects_contains(aabb, sdf);

            // contains
            if (std::get<1>(intersection)){
                structure[i] = is_leaf_flag;
                return;
            } 
            
            // intersects
            if (std::get<0>(intersection)){
                new_sdfs.push_back(sdf);
            }
        }
    }

    if (new_sdfs.empty()){
        structure[i] = is_leaf_flag;
        return;
    }      

    if (is_leaf(aabb, new_sdfs, camera)){
        uint32_t brick_i = create_brick(aabb, compose::union_t<3>(new_sdfs));
        structure[i] = is_leaf_flag | brick_i;
        return;
    } 

    structure[i] = structure.size();
    for (uint8_t octant = 0; octant < 8; octant++){
        structure.push_back(null_node);
    }

    // TODO: this sucks
    std::vector<std::weak_ptr<sdf3_t>> new_weak_sdfs;
    for (auto sdf_ptr : new_sdfs){
        new_weak_sdfs.push_back(sdf_ptr);
    }

    for (uint8_t octant = 0; octant < 8; octant++){
        vec4_t new_aabb = aabb;
        new_aabb[3] /= 2;

        if (octant & 1) new_aabb[0] += new_aabb[3];
        if (octant & 2) new_aabb[1] += new_aabb[3];
        if (octant & 4) new_aabb[2] += new_aabb[3];

        paint(structure[i] + octant, new_aabb, new_weak_sdfs, camera);
    }
}

bool 
octree_t::is_leaf(
    const vec4_t & aabb, 
    const std::vector<std::shared_ptr<sdf3_t>> & sdfs,
    std::shared_ptr<camera_t> camera
){
    vec3_t c = vec3_t(aabb[0], aabb[1], aabb[2]) + vec3_t(aabb[3] / 2);
    double d = (c - camera->get_position()).norm();

    if (aabb[3] <= std::max(hyper::epsilon, hyper::sigma * d)){
        return true;
    }

    return false;
}