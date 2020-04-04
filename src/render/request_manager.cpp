#include "render/request_manager.h"

#include <iostream>
#include "core/blaspheme.h"
#include <set>
#include "sdf/compose.h"
#include "sdf/mutate.h"
#include "maths/mat.h"
#include "core/constant.h"
#include "render/painter.h"
#include <ctime>

request_manager_t::request_manager_t(
    VmaAllocator allocator, std::shared_ptr<device_t> device,
    const std::vector<std::weak_ptr<substance_t>> & substances, 
    const std::vector<VkDescriptorSet> & desc_sets, VkCommandPool pool, VkQueue queue,
    u32vec2_t work_group_count, uint32_t work_group_size
){
    this->device = device->get_device();
    this->pool = pool;
    this->queue = queue;
    this->substances = substances;
    this->work_group_count = work_group_count;
    this->work_group_size = work_group_size;

    octree_buffer = std::make_unique<buffer_t<octree_node_t>>(
        allocator, device, work_group_count[0] * work_group_count[1] * work_group_size,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );

    substance_buffer = std::make_unique<buffer_t<substance_t::data_t>>(
        allocator, device, work_group_size, VMA_MEMORY_USAGE_CPU_TO_GPU
    );
    std::vector<substance_t::data_t> substance_data(work_group_size);
    substance_data[0].root = 0;
    substance_data[1].root = 8;
    substance_buffer->write(substance_data, 0, pool, queue);

    requests.resize(work_group_count[0] * work_group_count[1]);
    request_buffer = std::make_unique<buffer_t<request_t>>(
        allocator, device, requests.size(),
        VMA_MEMORY_USAGE_GPU_TO_CPU
    );
    request_buffer->write(requests, 0, pool, queue);

    persistent_state_buffer = std::make_unique<buffer_t<std::array<float, 8>>>(
        allocator, device, work_group_count[0] * work_group_count[1] * work_group_size,
        VMA_MEMORY_USAGE_GPU_ONLY
    );

    // write to descriptor sets
    std::vector<VkDescriptorBufferInfo> desc_buffer_infos = {
        octree_buffer->get_descriptor_info(),
        request_buffer->get_descriptor_info(),
        persistent_state_buffer->get_descriptor_info()
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

    f32vec4_t bounds(-hyper::rho, -hyper::rho, -hyper::rho, 2 * hyper::rho);

    std::vector<octree_node_t> initial_octree;
    initial_octree.resize(work_group_count[0] * work_group_count[1] * work_group_size);

    for (uint32_t i = 0; i < initial_octree.size(); i += work_group_size){
        for (uint32_t j = 0; j < substances.size(); j++){
            std::vector<octree_node_t> root_node(8);
            
            if (auto substance = substances[j].lock()){
                root_node = octree_node_t::create(bounds, substance->get_sdf());
            }

            for (uint32_t k = 0; k < 8; k++){
                initial_octree[i + j * 8 + k] = root_node[k];
            }
        }
    }
    
    octree_buffer->write(initial_octree, 0, pool, queue);
}

void
request_manager_t::handle_requests(){
    static request_t blank_request;

    vkDeviceWaitIdle(device); //TODO: remove this by baking in buffer updates

    request_buffer->read(requests);

    for (uint32_t x = 0; x < work_group_count[0]; x++){
        for (uint32_t y = 0; y < work_group_count[1]; y++){
            uint32_t work_group_id = x * work_group_count[1] + y;
            request_t r = requests[work_group_id];

            if (r.child != 0){
                std::vector<octree_node_t> new_node(8);
                if (auto substance = substances[r.objectID].lock()){
                    new_node = octree_node_t::create(r.aabb, substance->get_sdf());
                }

                octree_buffer->write(new_node, r.child, pool, queue);
                request_buffer->write({ blank_request }, work_group_id, pool, queue);
            }
        }
    }   
}
