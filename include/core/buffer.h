#ifndef BUFFER_H
#define BUFFER_H

#include "vk_mem_alloc.h"

#include <cstring>
#include <memory>

#include "core/command_buffer.h"
#include "core/device.h"
#include "maths/vec.h"

template<class T>
class buffer_t {
private:
    // fields
    bool is_host_visible;

    VmaAllocator allocator;
    std::shared_ptr<device_t> device;
    VmaAllocation allocation;
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize size;

    // helper methods
    void copy(
	    const buffer_t<T> & destination, uint64_t size, uint64_t offset, VkCommandPool pool, VkQueue queue
    ){
        if (size == 0){
            return;
        }

        command_buffer_t command_buffer(device->get_device(), pool, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        [&](VkCommandBuffer command_buffer){
            VkBufferCopy copy_region = {};
            copy_region.srcOffset = 0;
            copy_region.dstOffset = sizeof(T) * offset;
            copy_region.size = sizeof(T) * size;
            
            vkCmdCopyBuffer(command_buffer, buffer, destination.buffer, 1, &copy_region);
        });

        auto cmd_buf = command_buffer.get_command_buffer();
        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd_buf;
        submit_info.pNext = nullptr;
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = nullptr;
        submit_info.pWaitDstStageMask = nullptr;
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = nullptr;
            
        vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue); // remove! this line! updates need to be batched 
    }

public:
    // constructors and destructors
    buffer_t(VmaAllocator allocator, std::shared_ptr<device_t> device, uint64_t size, VmaMemoryUsage vma_usage){
        this->allocator = allocator;
        this->device = device;
        this->size = size;

        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = sizeof(T) * size;

        VmaAllocationCreateInfo alloc_create_info = {};
        alloc_create_info.usage = vma_usage;

        VmaAllocationInfo alloc_info = {};
        vmaCreateBuffer(allocator, &buffer_info, &alloc_create_info, &buffer, &allocation, &alloc_info);
    
        memory = alloc_info.deviceMemory;
        
        vkBindBufferMemory(device->get_device(), buffer, memory, 0); 

        is_host_visible = vma_usage != VMA_MEMORY_USAGE_GPU_ONLY;
    }

    ~buffer_t(){
        vmaDestroyBuffer(allocator, buffer, allocation);
    }

    // public methods
    void write(const std::vector<T> & source, uint64_t offset, VkCommandPool pool, VkQueue queue){
        if (source.empty()){
            return;
        }

        if (is_host_visible){
            void * mem_map;
            vkMapMemory(device->get_device(), memory, sizeof(T) * offset, sizeof(T) * source.size(), 0, &mem_map);
                std::memcpy(mem_map, source.data(), sizeof(T) * source.size());
            vkUnmapMemory(device->get_device(), memory);
        
        } else {
            buffer_t<T> staging_buffer(
                allocator, device, source.size(),
                VMA_MEMORY_USAGE_CPU_ONLY
            );

            staging_buffer.write(source, 0, pool, queue);
            staging_buffer.copy(*this, source.size(), offset, pool, queue); 
        }
    }

    void read(std::vector<T> & destination) {
        if (destination.empty() || !is_host_visible){
            return;
        }

        void * mem_map;
        vkMapMemory(device->get_device(), memory, 0, sizeof(T) * destination.size(), 0, &mem_map);
            std::memcpy(destination.data(), mem_map, sizeof(T) * destination.size());
        vkUnmapMemory(device->get_device(), memory);
    }

    VkDescriptorBufferInfo get_descriptor_info() const {
        VkDescriptorBufferInfo desc_buffer_info = {};
        desc_buffer_info.buffer = buffer;
        desc_buffer_info.offset = 0;
        desc_buffer_info.range  = sizeof(T) * size;
        return desc_buffer_info;
    }
};

#endif
