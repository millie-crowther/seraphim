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

    std::vector<std::shared_ptr<sdf3_t>> initial_sdfs;
    for (auto sdf_ptr : sdfs){
        if (auto sdf = sdf_ptr.lock()){
            initial_sdfs.push_back(sdf);
        }
    }
    initial_octree[0] = octree_node_t(f32vec3_t(-hyper::rho), 0, initial_sdfs);

    octree_buffer->copy(initial_octree.data(), sizeof(initial_octree), 0, pool, queue);
}

void
request_manager_t::handle_requests(){
    static std::array<request_t, max_requests_size> requests;
    static request_t blank_request;

    vkDeviceWaitIdle(device); //TODO: remove this by baking in buffer updates

    request_buffer->read(requests.data(), sizeof(requests));

    std::vector<std::shared_ptr<sdf3_t>> strong_sdfs;
    for (auto sdf_ptr : sdfs){
        if (auto sdf = sdf_ptr.lock()){
            strong_sdfs.push_back(sdf);
        }
    }

    for (uint16_t i = 0; i < requests.size(); i++){
        request_t r = requests[i];

        if ((r.child_24_depth_8 >> 24) != 0xFF){
            octree_node_t new_node(r.x, r.child_24_depth_8 >> 24, strong_sdfs);

            uint32_t child_index = r.child_24_depth_8 & 0xFFFFFF;
            octree_buffer->copy(&new_node, sizeof(octree_node_t), sizeof(octree_node_t) * child_index, pool, queue);

            request_buffer->copy(&blank_request, sizeof(request_t), sizeof(request_t) * i, pool, queue);
        }
    }    
}