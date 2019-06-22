#include "render/octree.h"

#include <iostream>
#include "core/blaspheme.h"
#include <set>
#include "sdf/compose.h"
#include "sdf/mutate.h"
#include "maths/mat.h"

constexpr uint32_t octree_t::null_node;

octree_t::octree_t(
    VmaAllocator allocator, VkCommandPool pool, VkQueue queue, double render_distance, 
    const std::vector<std::weak_ptr<sdf3_t>> & sdfs, 
    const std::vector<VkDescriptorSet> & desc_sets
){

    // TODO: remove this! its only here because the zero index is 
    //       regarded as an empty volume in the shader 
    structure.push_back(null_node); 
    gpu_bricks.emplace_back();

    std::vector<std::shared_ptr<sdf3_t>> strong_sdfs;
    for (auto sdf_ptr : sdfs){
        if (auto sdf = sdf_ptr.lock()){
            strong_sdfs.push_back(sdf);
        }
    }

    universal_aabb = vec4_t(-render_distance);
    universal_aabb[3] = render_distance * 2;
    paint(0, universal_aabb, strong_sdfs);

    uint32_t size = sizeof(uint32_t) * max_structure_size + sizeof(f32vec4_t) * max_geometry_size;
    buffer = std::make_unique<buffer_t>(
        allocator, size,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );

    // copy to buffer
    buffer->copy(pool, queue, structure.data(), sizeof(uint32_t) * structure.size(), 0);
    buffer->copy(pool, queue, gpu_bricks.data(),  sizeof(f32vec4_t) * gpu_bricks.size(), sizeof(uint32_t) * max_structure_size);

    // int redundant_nodes = 0;
    // for (int i = 1; i < structure.size(); i += 8){

    // }

    std::cout << "octree size: " << structure.size() << std::endl;
    std::cout << "gpu_bricks size: " << gpu_bricks.size() << std::endl;

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

uint32_t 
octree_t::create_brick(const vec3_t & x, const sdf3_t & sdf){
    gpu_bricks.emplace_back();
    bricks.emplace_back(x, texture_manager, sdf, &gpu_bricks[gpu_bricks.size() - 1]);
    return gpu_bricks.size() - 1;
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
    if (p < 0 && d >= aabb[3] / 2){
        return std::make_tuple(false, true);
    }

    // intersection check precise
    if (d <= aabb[3] / 2){
        return std::make_tuple(true, false);
    }

    return std::make_tuple(false, false);
}

void 
octree_t::paint(uint32_t i, const vec4_t & aabb, const std::vector<std::shared_ptr<sdf3_t>> & sdfs){
    bool is_leaf = aabb[3] <= 0.125;

    std::vector<std::shared_ptr<sdf3_t>> new_sdfs;

    for (auto sdf : sdfs){
        auto intersection = intersects_contains(aabb, sdf);

        if (std::get<1>(intersection)){
            // contains
            structure[i] = is_leaf_flag;
            return;
        } else if (std::get<0>(intersection)){
            new_sdfs.push_back(sdf);
        }
    }

    if (new_sdfs.empty()){
        structure[i] = is_leaf_flag;
        return;
    }      

    compose::union_t<3> u(new_sdfs);

    vec3_t c = vec3_t(aabb[0], aabb[1], aabb[2]) + vec3_t(aabb[3] / 2.0f);

    if (is_leaf){
        structure[i] = is_leaf_flag | create_brick(c, u);
        return;
    } 

    auto f = [&](const vec3_t & x){ return u.normal(x); };
    mat3_t j = mat3_t::jacobian(f, c, aabb[3] / 4.0);

    if (j.frobenius_norm() < constant::epsilon){
        structure[i] = is_leaf_flag | create_brick(c, u);
        return;
    }

    structure[i] = structure.size();
    for (uint8_t octant = 0; octant < 8; octant++){
        structure.push_back(null_node);
    }

    for (uint8_t octant = 0; octant < 8; octant++){
        vec4_t new_aabb = aabb;
        new_aabb[3] /= 2;

        if (octant & 1) new_aabb[0] += new_aabb[3];
        if (octant & 2) new_aabb[1] += new_aabb[3];
        if (octant & 4) new_aabb[2] += new_aabb[3];

        paint(structure[i] + octant, new_aabb, new_sdfs);
    }
}
