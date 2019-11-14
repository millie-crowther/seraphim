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
    const std::vector<std::weak_ptr<sdf3_t>> & sdfs, 
    const std::vector<VkDescriptorSet> & desc_sets, VkCommandPool pool, VkQueue queue,
    u32vec2_t work_group_count, uint32_t work_group_size
){
    this->device = device->get_device();
    this->pool = pool;
    this->queue = queue;
    this->sdfs = sdfs;
    this->work_group_count = work_group_count;
    this->work_group_size = work_group_size;

    octree_buffer = std::make_unique<buffer_t<std::array<octree_data_t, 8>>>(
        allocator, device, work_group_count[0] * work_group_count[1] * work_group_size,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );

    requests.resize(work_group_count[0] * work_group_count[1]);
    request_buffer = std::make_unique<buffer_t<request_t>>(
        allocator, device, requests.size(),
        VMA_MEMORY_USAGE_GPU_TO_CPU
    );
    request_buffer->write(requests, 0, pool, queue);

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

    std::vector<std::shared_ptr<sdf3_t>> initial_sdfs;
    for (auto sdf_ptr : sdfs){
        if (auto sdf = sdf_ptr.lock()){
            initial_sdfs.push_back(sdf);
        }
    }

    std::array<octree_data_t, 8> root_node = octree_data_t::create(f32vec3_t(-hyper::rho), 0, initial_sdfs);
    std::vector<std::array<octree_data_t, 8>> initial_octree;
    initial_octree.resize(work_group_count[0] * work_group_count[1] * work_group_size);

    for (uint32_t i = 0; i < initial_octree.size(); i += work_group_size){
        initial_octree[i] = root_node;
    }
    
    octree_buffer->write(initial_octree, 0, pool, queue);
}

void
request_manager_t::handle_requests(){
    static request_t blank_request;

    vkDeviceWaitIdle(device); //TODO: remove this by baking in buffer updates

    request_buffer->read(requests);

    std::vector<std::shared_ptr<sdf3_t>> strong_sdfs;
    for (auto sdf_ptr : sdfs){
        if (auto sdf = sdf_ptr.lock()){
            strong_sdfs.push_back(sdf);
        }
    }

    for (uint32_t x = 0; x < work_group_count[0]; x++){
        for (uint32_t y = 0; y < work_group_count[1]; y++){
            uint32_t work_group_id = x * work_group_count[1] + y;
            request_t r = requests[work_group_id];

            if (r.child != 0){
                std::array<octree_data_t, 8> new_node = octree_data_t::create(r.x, r.depth, strong_sdfs);

                octree_buffer->write({ new_node }, r.child, pool, queue);
                request_buffer->write({ blank_request }, work_group_id, pool, queue);
            }
        }
    }   
}