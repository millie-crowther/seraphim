#include "engine.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <set>
#include <algorithm>
#include <fstream>
#include "vertex.h"
#include <cstring>
#include "vk_utils.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

constexpr int engine_t::frames_in_flight;

const std::vector<const char *> validation_layers = {
    "VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<vertex_t> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
};

void 
window_resize_callback(GLFWwindow * window, int width, int height){
    void * data = glfwGetWindowUserPointer(window);
    engine_t * engine = reinterpret_cast<engine_t *>(data);
    engine->window_resize(width, height);
}

void
engine_t::window_resize(int w, int h){
    recreate_swapchain();
}

engine_t::engine_t(bool is_debug){
    this->is_debug = is_debug;

    if (is_debug){
	    std::cout << "Running in debug mode." << std::endl;
    } else {
	    std::cout << "Running in release mode." << std::endl;
    }
}

void
engine_t::init(){
    // initialise GLFW
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(640, 480, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, static_cast<void *>(this));
    glfwSetWindowSizeCallback(window, window_resize_callback);   

    // initialise vulkan
    create_instance();

    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

    std::cout << "Available extensions:" << std::endl;
    for (const auto& extension : extensions) {
	    std::cout << "\t" << extension.extensionName << std::endl;
    }

    if (is_debug){
	    if (!setup_debug_callback()){
	        throw std::runtime_error("Error: Failed to setup debug callback.");
	    }
    }

    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
	    throw std::runtime_error("Error: Failed to create window surface.");
    }

    physical_device = select_device();
    if (physical_device == VK_NULL_HANDLE){
	    throw std::runtime_error("Error: Couldn't find an appropriate GPU.");
    }

    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    std::cout << "Chosen physical device: " << properties.deviceName << std::endl;

    if (!create_logical_device()){
	throw std::runtime_error("Error: Couldn't create logical device.");
    }

    // initialise vulkan memory allocator
    VmaAllocatorCreateInfo allocator_info = {};
    allocator_info.physicalDevice = physical_device;
    allocator_info.device = device;
    
    vmaCreateAllocator(&allocator_info, &allocator);

    if (!create_swapchain()){
	throw std::runtime_error("Error: Couldn't create swapchain.");
    }

    if (!create_render_pass()){
	throw std::runtime_error("Error: Couldn't create render pass.");
    }

    if (!create_descriptor_set_layout()){
	throw std::runtime_error("Error: Couldn't create descriptor set layout.");
    }

    if (!create_graphics_pipeline()){
	throw std::runtime_error("Error: Failed to create graphics pipeline.");
    }

    if (!create_command_pool()){
	throw std::runtime_error("Error: Failed to create command pool.");
    }

    if (!create_depth_resources()){
        throw std::runtime_error("Error: Failed to create depth resources.");
    }

    if (!create_framebuffers()){
	throw std::runtime_error("Error: Failed to create framebuffers.");
    }


    VkDeviceSize size = sizeof(indices[0]) * indices.size();

    // create index buffer
    index_buffer = new buffer_t(
        physical_device, device, size,
	VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    index_buffer->copy(command_pool, graphics_queue, (void *) indices.data(), size);
    
    // create vertex buffer
    size = sizeof(vertex_t) * vertices.size();
    vertex_buffer = new buffer_t(
        physical_device, device, size,
	VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    vertex_buffer->copy(command_pool, graphics_queue, (void *) vertices.data(), size);

    // create uniform buffers
    size = sizeof(UniformBufferObject);
    uniform_buffers.resize(swapchain_images.size());
    for (int i = 0; i < swapchain_images.size(); i++){
        uniform_buffers[i] = new buffer_t(
            physical_device, device, size,
	    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
    }
 
    if (!create_descriptor_pool()){
	throw std::runtime_error("Error: Failed to create descriptor pool.");
    }

    if (!create_command_buffers()){
        throw std::runtime_error("Error: Failed to create command buffers.");
    }

    if (!create_sync()){
        throw std::runtime_error("Error: Failed to create synchronisation primitives.");
    }
}

std::vector<char>
engine_t::load_file(std::string filename){
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()){
	return std::vector<char>();
    }

    size_t filesize = file.tellg();    
    std::vector<char> buffer(filesize);

    file.seekg(0);
    file.read(buffer.data(), filesize);
    file.close();

    return buffer;
}

bool
engine_t::create_logical_device(){
    int graphics = get_graphics_queue_family(physical_device);
    int present = get_present_queue_family(physical_device);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<int> unique_queue_families = { graphics, present };
    
    float queue_priority = 1.0f;
    for (int queue_family : unique_queue_families){
	VkDeviceQueueCreateInfo queue_create_info = {};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueFamilyIndex = queue_family;
	queue_create_info.queueCount = 1;
	queue_create_info.pQueuePriorities = &queue_priority;
	queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features = {};

    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());

    create_info.pEnabledFeatures = &device_features;

    
    create_info.enabledExtensionCount = device_extensions.size();
    create_info.ppEnabledExtensionNames = device_extensions.data();

    if (is_debug) {
	create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
	create_info.ppEnabledLayerNames = validation_layers.data();
    } else {
	create_info.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS){
	return false;
    }

    vkGetDeviceQueue(device, graphics, 0, &graphics_queue);
    vkGetDeviceQueue(device, present, 0, &present_queue);

    return true;
}

VkExtent2D
engine_t::select_swap_extent(){
     VkSurfaceCapabilitiesKHR capabilities;
     vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

     // check if we need to supply width and height
     if (capabilities.currentExtent.width == ~((uint32_t) 0)){
          int width, height;
          glfwGetFramebufferSize(window, &width, &height);
	  VkExtent2D extents = { (uint32_t) width, (uint32_t) height };
	  
	  extents.width = std::max(
	      capabilities.minImageExtent.width, 
	      std::min(extents.width, capabilities.maxImageExtent.width)
	  );
	  extents.height = std::max(
	      capabilities.minImageExtent.height, 
	      std::min(extents.height, capabilities.maxImageExtent.height)
	  );
          
	  return extents;
     } else {
	  return capabilities.currentExtent;
     }
}

VkPresentModeKHR
engine_t::select_present_mode(){
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &count, nullptr);
    std::vector<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &count, modes.data());

    if (std::find(modes.begin(), modes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != modes.end()){
	return VK_PRESENT_MODE_MAILBOX_KHR;
    }

    if (std::find(modes.begin(), modes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR) != modes.end()){
	return VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

bool
engine_t::create_framebuffers(){
    swapchain_framebuffers.resize(swapchain_images.size());

    for (int i = 0; i < swapchain_framebuffers.size(); i++){
        std::array<VkImageView, 2> attachments = {
	        swapchain_images[i]->get_image_view(),
            depth_image->get_image_view()
	    };

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = attachments.size();
        framebuffer_info.pAttachments = attachments.data();
        framebuffer_info.width = swapchain_extents.width;
        framebuffer_info.height = swapchain_extents.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(
            device, &framebuffer_info, nullptr, &swapchain_framebuffers[i]) != VK_SUCCESS
        ){
                return false;
        }
    }

    return true;
}

bool
engine_t::create_swapchain(){
    VkSurfaceFormatKHR format = select_surface_format();
    VkPresentModeKHR mode = select_present_mode();
    VkExtent2D extents = select_swap_extent();

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);
    uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount != 0 && image_count > capabilities.maxImageCount){
	    image_count = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = format.format;
    create_info.imageColorSpace = format.colorSpace;
    create_info.imageExtent = extents;
    create_info.imageArrayLayers = 1;

    // if you dont wanna draw to image directly VK_IMAGE_USAGE_TRANSFER_DST_BIT
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t families[2] = { 
        (uint32_t) get_graphics_queue_family(physical_device),
        (uint32_t) get_present_queue_family(physical_device)
    };

    if (families[0] != families[1]){
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
	    create_info.pQueueFamilyIndices = families;
    } else {
	    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
	    create_info.pQueueFamilyIndices = nullptr;
    }

    create_info.preTransform = capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    create_info.presentMode = mode;
    create_info.clipped = VK_TRUE;

    // will need to update this field if creating a new swap chain e.g. for resized window
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain) != VK_SUCCESS){
	return false;
    }

    uint32_t count = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);

    VkImage swapchain_imgs[count];
    vkGetSwapchainImagesKHR(device, swapchain, &count, swapchain_imgs);
  
    swapchain_images.resize(count);
    for (int i = 0; i < count; i++){
        swapchain_images[i] = new image_t(
            physical_device, device, swapchain_imgs[i], format.format, VK_IMAGE_ASPECT_COLOR_BIT
        );
    }

    swapchain_extents = extents;

    return true;
}

bool 
engine_t::create_descriptor_pool(){
    VkDescriptorPoolSize pool_size = {};
    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = static_cast<uint32_t>(swapchain_images.size());

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    pool_info.maxSets = static_cast<uint32_t>(swapchain_images.size());

    if (vkCreateDescriptorPool(device, &pool_info, nullptr, &desc_pool) != VK_SUCCESS){
	return false;
    }

    if (!create_descriptor_sets()){
	return false;
    }

    return true;
}

bool 
engine_t::create_descriptor_sets(){
    std::vector<VkDescriptorSetLayout> layouts(swapchain_images.size(), descriptor_layout);

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = desc_pool;
    alloc_info.descriptorSetCount = static_cast<uint32_t>(swapchain_images.size());
    alloc_info.pSetLayouts = layouts.data();

    desc_sets.resize(swapchain_images.size());
    if (vkAllocateDescriptorSets(device, &alloc_info, desc_sets.data()) != VK_SUCCESS){
	return false;
    }

    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.offset = 0;
    buffer_info.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet desc_write = {};
    desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_write.dstBinding = 0;
    desc_write.dstArrayElement = 0;
    desc_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    desc_write.descriptorCount = 1;
    desc_write.pBufferInfo = &buffer_info;
    desc_write.pImageInfo = nullptr;
    desc_write.pTexelBufferView = nullptr;

    for (int i = 0; i < swapchain_images.size(); i++){
	buffer_info.buffer = uniform_buffers[i]->get_buffer();
	desc_write.dstSet = desc_sets[i];

	vkUpdateDescriptorSets(device, 1, &desc_write, 0, nullptr);
    }

    return true;
}

bool
engine_t::create_descriptor_set_layout(){
    VkDescriptorSetLayoutBinding ubo_layout = {};
    ubo_layout.binding = 0;
    ubo_layout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout.descriptorCount = 1;
    ubo_layout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout.pImmutableSamplers = nullptr;
    
    VkDescriptorSetLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 1;
    layout_info.pBindings = &ubo_layout;

    VkResult result = vkCreateDescriptorSetLayout(
        device, &layout_info, nullptr, &descriptor_layout
    );
 
    if (result != VK_SUCCESS){
	return false;
    }

    return true;
}

bool
engine_t::create_depth_resources(){
    VkFormat depth_format = image_t::find_depth_format(physical_device);

    depth_image = new image_t(
        physical_device, device, swapchain_extents.width, swapchain_extents.height, depth_format,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT
    );
    depth_image->transition_image_layout(
        command_pool, graphics_queue, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    );
    return true;
}

bool
engine_t::create_render_pass(){
    VkAttachmentDescription colour_attachment = {};
    colour_attachment.format = swapchain_images[0]->get_format();
    colour_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colour_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colour_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colour_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colour_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colour_attachment_ref = {};
    colour_attachment_ref.attachment = 0;
    colour_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depth_attachment = {};
    depth_attachment.format = image_t::find_depth_format(physical_device);
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref = {};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colour_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
                             | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colour_attachment, depth_attachment };

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = attachments.size();
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    return vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass) == VK_SUCCESS;
}

bool
engine_t::create_command_pool(){
    // create command pool
    VkCommandPoolCreateInfo command_pool_info = {};
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.queueFamilyIndex = get_graphics_queue_family(physical_device);
    command_pool_info.flags = 0;

    if (vkCreateCommandPool(device, &command_pool_info, nullptr, &command_pool) != VK_SUCCESS){
    	return false;
    }

    return true;
}

bool
engine_t::create_command_buffers(){
    // create command buffers
    command_buffers.resize(swapchain_framebuffers.size());
    
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = (uint32_t) command_buffers.size();

    if (vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data()) != VK_SUCCESS){
	return false;
    }

    for (int i = 0; i < command_buffers.size(); i++){
        VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	if (vkBeginCommandBuffer(command_buffers[i], &begin_info) != VK_SUCCESS){
            return false;
	}

	VkRenderPassBeginInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = render_pass;
	render_pass_info.framebuffer = swapchain_framebuffers[i];
	render_pass_info.renderArea.offset = {0, 0};
	render_pass_info.renderArea.extent = swapchain_extents;

	std::array<VkClearValue, 2> clear_values = {};
        clear_values[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
        clear_values[1].depthStencil = { 1.0f, 0 };

	render_pass_info.clearValueCount = clear_values.size();
	render_pass_info.pClearValues = clear_values.data();

	vkCmdBeginRenderPass(command_buffers[i], &render_pass_info,VK_SUBPASS_CONTENTS_INLINE);
	    vkCmdBindPipeline(
		command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline
	    );

            VkBuffer vertex_buffers[1] = { vertex_buffer->get_buffer() };
            VkDeviceSize offsets[1] = { 0 };
	    vkCmdBindVertexBuffers(command_buffers[i], 0, 1, vertex_buffers, offsets);
            vkCmdBindIndexBuffer(
                command_buffers[i], index_buffer->get_buffer(), 0, VK_INDEX_TYPE_UINT32
            );
            vkCmdBindDescriptorSets(
		command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout,
		0, 1, &desc_sets[i], 0, nullptr
	    );

	    vkCmdDrawIndexed(command_buffers[i], (uint32_t) indices.size(), 1, 0, 0, 0);
	vkCmdEndRenderPass(command_buffers[i]);

        if (vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS){
	    return false;
	}
    }

    return true;
}

VkSurfaceFormatKHR
engine_t::select_surface_format(){
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count, formats.data());
    
    // check if all formats supported
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED){
	return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }

    // check for preferred
    for (auto available_format : formats){
	if (
	    available_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
	    available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
	){
	    return available_format;
	}
    }

    // default
    return formats[0];
}

bool
engine_t::check_validation_layers(){
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (auto layer : validation_layers){
        bool found = false;
	
	for (auto layer_property : available_layers){ 
            if (layer == std::string(layer_property.layerName)){
		found = true;
		break;
	    }
	}

	if (!found){
	    return false;
	}
    }

    return true;
}

std::vector<const char *>
engine_t::get_required_extensions(){
    uint32_t extension_count = 0;
    const char ** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&extension_count);

    std::vector<const char *> req_ext(glfw_extensions, glfw_extensions + extension_count);
    if (is_debug){
	req_ext.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    return req_ext;
}

int
engine_t::get_graphics_queue_family(VkPhysicalDevice phys_device){
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> q_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, q_families.data());
   
    for (int i = 0; i < queue_family_count; i++){
	if (q_families[i].queueCount > 0 && q_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
	    return i;
	}
    }

    return -1;    
}

int 
engine_t::get_present_queue_family(VkPhysicalDevice phys_device){
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> q_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, q_families.data());
   
    for (int i = 0; i < queue_family_count; i++){
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(phys_device, i, surface, &present_support);

	if (q_families[i].queueCount > 0 && present_support){
	    return i;
	}
    }

    return -1;    
}

VkShaderModule
engine_t::create_shader_module(const std::vector<char>& code, bool * success){
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO; 
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS){
	*success = false;
    }
    return shader_module;
}

bool 
engine_t::create_graphics_pipeline(){
    auto vertex_shader_code = load_file("../src/shaders/shader.vert");
    auto fragment_shader_code = load_file("../src/shaders/shader.frag");

    if (vertex_shader_code.size() == 0 || fragment_shader_code.size() == 0){
	return false;
    }

    bool success = true;
    VkShaderModule vert_shader_module = create_shader_module(vertex_shader_code, &success);
    VkShaderModule frag_shader_module = create_shader_module(fragment_shader_code, &success);
    
    if (!success){
	return false;
    }

    VkPipelineShaderStageCreateInfo vert_create_info = {}; 
    vert_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_create_info.module = vert_shader_module;
    vert_create_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_create_info = {}; 
    frag_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_create_info.module = frag_shader_module;
    frag_create_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[2] = {
	vert_create_info,
	frag_create_info
    };

    auto vert_desc = vertex_t::get_binding_description();
    auto attr_desc = vertex_t::get_attr_descriptions();

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &vert_desc;
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(
	attr_desc.size()
    );
    vertex_input_info.pVertexAttributeDescriptions = attr_desc.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float) swapchain_extents.width;
    viewport.height = (float) swapchain_extents.height;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapchain_extents;

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo raster_info = {};
    raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster_info.depthClampEnable = VK_FALSE;
    raster_info.rasterizerDiscardEnable = VK_FALSE;
    raster_info.polygonMode = VK_POLYGON_MODE_FILL;
    raster_info.lineWidth = 1.0f;
    raster_info.cullMode = VK_CULL_MODE_BACK_BIT;
    raster_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    raster_info.depthBiasEnable = VK_FALSE;
    raster_info.depthBiasConstantFactor = 0;
    raster_info.depthBiasClamp = 0;
    raster_info.depthBiasSlopeFactor = 0;
    
    VkPipelineMultisampleStateCreateInfo multisample_info = {};
    multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_info.sampleShadingEnable = VK_FALSE;
    multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_info.minSampleShading = 1.0f;
    multisample_info.pSampleMask = nullptr;
    multisample_info.alphaToCoverageEnable = VK_FALSE;
    multisample_info.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colour_blending = {};
    colour_blending.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
                                   | VK_COLOR_COMPONENT_G_BIT
                                   | VK_COLOR_COMPONENT_B_BIT
                                   | VK_COLOR_COMPONENT_A_BIT;
    colour_blending.blendEnable = VK_FALSE;
    colour_blending.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colour_blending.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colour_blending.colorBlendOp = VK_BLEND_OP_ADD;
    colour_blending.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colour_blending.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colour_blending.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colour_blend_info = {};
    colour_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colour_blend_info.logicOpEnable = VK_FALSE;
    colour_blend_info.logicOp = VK_LOGIC_OP_COPY;
    colour_blend_info.attachmentCount = 1;
    colour_blend_info.pAttachments = &colour_blending;
    colour_blend_info.blendConstants[0] = 0.0f;
    colour_blend_info.blendConstants[1] = 0.0f;
    colour_blend_info.blendConstants[2] = 0.0f;
    colour_blend_info.blendConstants[3] = 0.0f;
    const VkPipelineColorBlendStateCreateInfo colour_blend_const = colour_blend_info;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_layout;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(
	device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS
    ){
	return false;
    }

    VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS; 
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f; 
    depth_stencil.maxDepthBounds = 1.0f;
    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.front = {};
    depth_stencil.back = {};

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &raster_info;
    pipeline_info.pMultisampleState = &multisample_info;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pColorBlendState = &colour_blend_const;
    pipeline_info.pDynamicState = nullptr;
    pipeline_info.layout = pipeline_layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(
	device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline) != VK_SUCCESS
    ){
	return false;
    }

    vkDestroyShaderModule(device, vert_shader_module, nullptr);
    vkDestroyShaderModule(device, frag_shader_module, nullptr);

    return true;
}

bool
engine_t::has_adequate_swapchain(VkPhysicalDevice physical_device){
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count, nullptr);
    if (count == 0){
	return false;
    }

    count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &count, nullptr);
    if (count == 0){
	return false;
    }

    return true;
}

bool
engine_t::is_suitable_device(VkPhysicalDevice phys_device){
    // check that gpu isnt integrated
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(phys_device, &properties);
    if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
        return false;
    }

    // check device can do geometry shaders
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(phys_device, &features);
    if (!features.geometryShader){
	return false;
    }

    // check device has at least one graphics queue family
    if (get_graphics_queue_family(phys_device) == -1){
	return false;
    }

    if (get_present_queue_family(phys_device) == -1){
	return false;
    }

    for (auto extension : device_extensions){
	if (!device_has_extension(phys_device, extension)){
            return false;
	}
    }

    if (!has_adequate_swapchain(phys_device)){
	return false;
    }

    return true;
}

bool
engine_t::device_has_extension(VkPhysicalDevice phys_device, const char * extension){
    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(phys_device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(
        phys_device, nullptr, &extension_count, available_extensions.data()
    );

    for (auto available_extension : available_extensions){
	if (std::string(extension) == std::string(available_extension.extensionName)){
	    return true;
	}
    }

    return false;
}

VkPhysicalDevice
engine_t::select_device(){
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    if (device_count == 0){
	return VK_NULL_HANDLE;
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    
    for (auto phys_device : devices){
	if (is_suitable_device(phys_device)){
            return phys_device;
	}
    }

    return VK_NULL_HANDLE;
}

void
engine_t::create_instance(){
    if (is_debug && !check_validation_layers()){
	throw std::runtime_error("Requested validation layers not available.");
    }

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Blaspheme";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    auto required_extensions = get_required_extensions(); 

    create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    create_info.ppEnabledExtensionNames = required_extensions.data();

    std::cout << "Enabled extensions: " << std::endl;
    for (int i = 0; i < create_info.enabledExtensionCount; i++){
        std::cout << "\t" << create_info.ppEnabledExtensionNames[i] << std::endl;
    }

    if (is_debug){
	create_info.ppEnabledLayerNames = validation_layers.data();
	create_info.enabledLayerCount = validation_layers.size();
    } else {
        create_info.enabledLayerCount = 0;
    }

    std::cout << "Enabled validation layers: "  << std::endl;
    for (int i = 0; i < create_info.enabledLayerCount; i++){
        std::cout << "\t" << create_info.ppEnabledLayerNames[i] << std::endl;
    }

    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS){
	throw std::runtime_error("Failed to create Vulkan instance!");
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL 
debug_callback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT obj_type,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char * layer_prefix,
    const char * msg,
    void * user_data
){
    std::cout << "Validation layer debug message: " << msg << std::endl;
    return VK_FALSE;
}

bool
engine_t::setup_debug_callback(){
    if (!is_debug){
	return true;
    }

    VkDebugReportCallbackCreateInfoEXT create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT 
                      | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    create_info.pfnCallback = debug_callback;

    //load in function address, since its an extension
    auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(
        instance, "vkCreateDebugReportCallbackEXT"
    );
    if (func == nullptr){
        return false;
    }

    if (func(instance, &create_info, nullptr, &callback) != VK_SUCCESS){
	return false;
    }	

    return true;
}

bool
engine_t::create_sync(){
    image_available_semas.resize(frames_in_flight);
    render_finished_semas.resize(frames_in_flight);
    in_flight_fences.resize(frames_in_flight);

    VkSemaphoreCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
   
    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
 
    for (int i = 0; i < frames_in_flight; i++){
        if (vkCreateSemaphore(
            device, &create_info, nullptr, &image_available_semas[i]) != VK_SUCCESS
        ){
            return false;
        }

        if (vkCreateSemaphore(
            device, &create_info, nullptr, &render_finished_semas[i]) != VK_SUCCESS
        ){
            return false;
        }

        if (vkCreateFence(device, &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS){
            return false;
        }
    }

    return true;
}

void 
engine_t::update(){
    glfwPollEvents();
}

void
engine_t::recreate_swapchain(){
    vkDeviceWaitIdle(device);
  
    cleanup_swapchain();
    
    create_swapchain();
    create_render_pass();
    create_graphics_pipeline();
    create_depth_resources();
    create_framebuffers();
    create_command_buffers();
}

void 
engine_t::cleanup_swapchain(){
    delete depth_image;
    depth_image = nullptr;

    for (auto framebuffer : swapchain_framebuffers){
	vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    vkFreeCommandBuffers(
        device, command_pool, static_cast<uint32_t>(command_buffers.size()), command_buffers.data()
    );

    vkDestroyPipeline(device, graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
    vkDestroyRenderPass(device, render_pass, nullptr);

    for (int i = 0; i < swapchain_images.size(); i++){
        delete swapchain_images[i];
    }

    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void
engine_t::cleanup(){
    vkDeviceWaitIdle(device);

    vkDestroyDescriptorSetLayout(device, descriptor_layout, nullptr);

    for (auto buffer : uniform_buffers){
        delete buffer;
    }
    uniform_buffers.clear();

    cleanup_swapchain();
   
    delete vertex_buffer;
    vertex_buffer = nullptr;

    delete index_buffer;
    index_buffer = nullptr;

    for (int i = 0; i < frames_in_flight; i++){
        vkDestroySemaphore(device, image_available_semas[i], nullptr);
        vkDestroySemaphore(device, render_finished_semas[i], nullptr);
        vkDestroyFence(device, in_flight_fences[i], nullptr);
    }

    vkDestroyCommandPool(device, command_pool, nullptr);


    // destroy logical device
    vkDestroyDevice(device, nullptr);

    // destroy debug callback
    if (is_debug){
        auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(
            instance, "vkDestroyDebugReportCallbackEXT"
        );
	if (func != nullptr){
	    func(instance, callback, nullptr);
	}
    }
   
    vkDestroySurfaceKHR(instance, surface, nullptr);

    // destroy instance
    vkDestroyInstance(instance, nullptr);

    // destory GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool
engine_t::should_quit(){
    return glfwWindowShouldClose(window);
}

void
engine_t::update_uniform_buffers(uint32_t image_index){
    static auto start = std::chrono::high_resolution_clock::now();
    auto current = std::chrono::high_resolution_clock::now();
    
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
        current - start
    ).count();

    UniformBufferObject ubo = {};
    ubo.model = glm::rotate(
        glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)
    );

    ubo.view = glm::lookAt(glm::vec3(2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(
	glm::radians(45.0f), swapchain_extents.width / (float) swapchain_extents.height, 
	0.1f, 10.0f
    );

    ubo.proj[1][1] *= -1; // account for OpenGL weirdness

    uniform_buffers[image_index]->copy(
        command_pool, graphics_queue, (void *) &ubo, sizeof(ubo)
    );
}

void
engine_t::render(int current_frame){
    vkWaitForFences(device, 1, &in_flight_fences[current_frame], VK_TRUE, ~((uint64_t) 0));
    vkResetFences(device, 1, &in_flight_fences[current_frame]); 

    uint32_t image_index;
    vkAcquireNextImageKHR(
        device, swapchain, ~((uint64_t) 0), image_available_semas[current_frame], 
        VK_NULL_HANDLE, &image_index
    );
   
    update_uniform_buffers(image_index);

    VkSemaphore wait_semas[1] = { image_available_semas[current_frame] };
    VkPipelineStageFlags wait_stages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submit_info = {};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semas;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers[image_index];
    
    VkSemaphore signal_semas[1] = { render_finished_semas[current_frame] };
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semas;

    VkResult res = vkQueueSubmit(
        graphics_queue, 1, &submit_info, in_flight_fences[current_frame]
    );

    if (res == VK_ERROR_OUT_OF_DATE_KHR){
        //recreate_swapchain();
        return;
    } else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR){
        throw std::runtime_error("Error: Failed to submit to draw command buffer.");
    }

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semas;

    VkSwapchainKHR swapchains[1] = { swapchain };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;
    
    res = vkQueuePresentKHR(present_queue, &present_info);
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR){ 
        //recreate_swapchain();
    } else if (res != VK_SUCCESS){
        throw std::runtime_error("Error: Failed to present image.");
    }
}

void
engine_t::run(){
    init();	

    int current_frame = 0;
    while (!should_quit()){
	update();

        render(current_frame);
        current_frame = (current_frame + 1) % frames_in_flight;    
    }

    cleanup();
}
