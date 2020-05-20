#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>
#include <memory>

#include "core/command.h"
#include "core/device.h"
#include "maths/vec.h"

class buffer_t {
public:
    enum usage_t {
        device_local, host_to_device, device_to_host, host_local
    };

private:
    std::shared_ptr<device_t> device;
    VkBuffer buffer;
    VkDeviceMemory memory;
    uint32_t size;
    uint32_t binding;
    VkDescriptorBufferInfo desc_buffer_info;
    std::unique_ptr<buffer_t> staging_buffer;
    usage_t usage;
    std::vector<VkBufferCopy> updates;

public:
    // constructors and destructors
    buffer_t(uint32_t binding, std::shared_ptr<device_t> device, uint64_t size, usage_t usage);
    ~buffer_t();

    // public methods
    template<class F>
    void map(uint64_t offset, uint64_t size, const F & f){
        void * memory_map;
        vkMapMemory(device->get_device(), memory, offset, size, 0, &memory_map);
        f(memory_map);
        vkUnmapMemory(device->get_device(), memory);
    }

    template<class T>
    void write(const T & source, uint64_t offset){
        if (source.empty()){
            return;
        }

        uint32_t size = sizeof(typename T::value_type) * source.size();

        if (usage == host_local){
            map(offset, size, [&](void * memory_map){
                std::memcpy(memory_map, source.data(), size);
            });
        } else {
            staging_buffer->write(source, offset);

            VkBufferCopy buffer_copy;
            buffer_copy.srcOffset = offset;
            buffer_copy.dstOffset = offset;
            buffer_copy.size = size;
            updates.push_back(buffer_copy);
        }
    }

    void transfer(VkCommandBuffer command_buffer) const { 
        vkCmdCopyBuffer(command_buffer, staging_buffer->buffer, buffer, updates.size(), updates.data());
    }

    void flush(){
        updates.clear();
    }

    template<class T>
    void read(T & destination, uint64_t offset) {
        if (destination.empty()){
            return;
        }

        uint32_t size = sizeof(typename T::value_type) * destination.size();
        map(offset, size, [&](void * memory_map){
            std::memcpy(destination.data(), memory_map, size);
        });
    }
    
    VkWriteDescriptorSet get_write_descriptor_set(VkDescriptorSet descriptor_set) const;
    VkDescriptorSetLayoutBinding get_descriptor_set_layout_binding() const;

    VkBuffer get_buffer() const;

    static uint32_t find_memory_type(std::shared_ptr<device_t> device, uint32_t type_filter, VkMemoryPropertyFlags prop);
};

#endif
