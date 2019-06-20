#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "vk_mem_alloc.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct allocator_t {
    VmaAllocator vma_allocator;
    VkDevice device;
    VkPhysicalDevice physical_device;
    VkCommandPool pool;
    VkQueue queue;
};

#endif