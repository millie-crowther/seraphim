#include "render/request_manager.h"

#include <iostream>
#include "core/blaspheme.h"
#include <set>
#include "sdf/compose.h"
#include "sdf/mutate.h"
#include "maths/mat.h"
#include "core/constant.h"
#include "render/painter.h"

request_manager_t::request_manager_t(
    VmaAllocator allocator, std::shared_ptr<device_t> device,
    const std::vector<std::weak_ptr<sdf3_t>> & sdfs, 
    const std::vector<VkDescriptorSet> & desc_sets, VkCommandPool pool, VkQueue queue
){
    this->device = device->get_device();
    this->pool = pool;
    this->queue = queue;
    this->sdfs = sdfs;

    universal_aabb = vec4_t(-hyper::rho);
    universal_aabb[3] = hyper::rho * 2;

    // extra node at end is to allow shader to avoid branching
    octree_buffer = std::make_unique<buffer_t>(
        allocator, device, sizeof(octree_node_t) * octree_size,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );

    uint32_t request_size = sizeof(request_t) * max_requests_size;
    request_buffer = std::make_unique<buffer_t>(
        allocator, device, request_size,
        VMA_MEMORY_USAGE_GPU_TO_CPU
    );

    std::array<request_t, max_requests_size> initial_requests;
    request_buffer->copy(initial_requests.data(), sizeof(initial_requests), 0, pool, queue);

    // write to descriptor sets
    std::vector<VkDescriptorBufferInfo> desc_buffer_infos = {
        octree_buffer->get_descriptor_info(),
        request_buffer->get_descriptor_info()
    };

    VkWriteDescriptorSet write_desc_set = {};
    write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_desc_set.pNext = nullptr;
    write_desc_set.dstArrayElement = 0;
    write_desc_set.descriptorCount = 1;
    write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write_desc_set.pImageInfo = nullptr;
    write_desc_set.pTexelBufferView = nullptr;

    std::vector<VkWriteDescriptorSet> write_desc_sets;
    for (uint32_t i = 0; i < desc_sets.size(); i++){
        write_desc_set.dstSet = desc_sets[i];

        for (uint32_t j = 0; j < desc_buffer_infos.size(); j++){
            write_desc_set.dstBinding = j + 1;
            write_desc_set.pBufferInfo = &desc_buffer_infos[j];
            write_desc_sets.push_back(write_desc_set);
        }
    }

    vkUpdateDescriptorSets(device->get_device(), write_desc_sets.size(), write_desc_sets.data(), 0, nullptr);

    std::array<octree_node_t, octree_size> initial_octree;
    octree_node_t unused_node;
    node_t unused_node_data;
    unused_node_data.type = node_type_unused;
    unused_node.children.fill(unused_node_data);
    initial_octree.fill(unused_node);

    for (uint8_t octant = 0; octant < 8; octant++){
        vec4_t aabb = universal_aabb;
        aabb[3] /= 2;
        if (octant & 1) aabb[0] += aabb[3];
        if (octant & 2) aabb[1] += aabb[3];
        if (octant & 4) aabb[2] += aabb[3];
        initial_octree[0].children[octant] = create_node(aabb);
    }

    octree_buffer->copy(initial_octree.data(), sizeof(initial_octree), 0, pool, queue);
}

std::tuple<bool, bool> 
request_manager_t::intersects_contains(const vec4_t & aabb, std::shared_ptr<sdf3_t> sdf) const {
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

request_manager_t::node_t 
request_manager_t::create_node(const vec4_t & aabb){
    std::vector<std::shared_ptr<sdf3_t>> new_sdfs;

    node_t node;

    for (auto sdf_ptr : sdfs){
        if (auto sdf = sdf_ptr.lock()){
            auto intersection = intersects_contains(aabb, sdf);
            // contains
            if (std::get<1>(intersection)){
                node.type = node_type_empty;
                return node;
            }

            // intersects
            if (std::get<0>(intersection)){
                new_sdfs.push_back(sdf);
            }
        }
    }

    if (new_sdfs.empty()){
        node.type = node_type_empty;
        return node;
    }

    compose::union_t<3> sdf(new_sdfs);
    
    vec3_t c = vec3_t(aabb[0], aabb[1], aabb[2]) + vec3_t(aabb[3] / 2);

    double p = sdf.phi(c);
    p /= constant::sqrt3 * aabb[3];
    p += 0.5;
    p *= 255;
    p = std::max(0.0, std::min(p, 255.0)); 

    vec3_t n = sdf.normal(c);
    u8vec4_t colour = painter_t<3>().colour(c - n * sdf.phi(c));

    n = (n / 2 + 0.5) * 255;
    u8vec4_t normal(n[0], n[1], n[2], p);
    
    node.type = node_type_leaf;
    node.b = *reinterpret_cast<uint32_t *>(&normal);
    node.c = *reinterpret_cast<uint32_t *>(&colour);
    return node;
}

void 
request_manager_t::handle_request(const request_t & r){
    double size = hyper::rho / (1 << ((r.child_24_depth_8 >> 24) - 1)); 

    octree_node_t new_node;
    for (uint8_t octant = 0; octant < 8; octant++){
        vec4_t aabb(r.x[0], r.x[1], r.x[2], size);
        if (octant & 1) aabb[0] += aabb[3];
        if (octant & 2) aabb[1] += aabb[3];
        if (octant & 4) aabb[2] += aabb[3];
        new_node.children[octant] = create_node(aabb);
    }

    uint32_t child_index = r.child_24_depth_8 & 0xFFFFFF;
    octree_buffer->copy(&new_node, sizeof(octree_node_t), sizeof(octree_node_t) * child_index, pool, queue);
}

void
request_manager_t::handle_requests(){
    vkDeviceWaitIdle(device); //TODO: remove this by baking in buffer updates

    static std::array<request_t, max_requests_size> requests;
    request_buffer->read(requests.data(), sizeof(requests));

    static request_t blank_request;

    for (uint16_t i = 0; i < requests.size(); i++){
        if ((requests[i].child_24_depth_8 >> 24) > 0){
            handle_request(requests[i]);
            request_buffer->copy(&blank_request, sizeof(request_t), sizeof(request_t) * i, pool, queue);
        }
    }    
}