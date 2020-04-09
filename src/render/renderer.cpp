#include "render/renderer.h"

#include "sdf/primitive.h"
#include "ui/resources.h"
#include "render/texture.h"

#include <chrono>
#include <ctime>
#include <stdexcept>

renderer_t::renderer_t(
    VmaAllocator allocator, std::shared_ptr<device_t> device,
    VkSurfaceKHR surface, std::shared_ptr<window_t> window,
    std::shared_ptr<camera_t> test_camera
){
    this->allocator = allocator;
    this->device = device;
    this->surface = surface;

    work_group_count = u32vec2_t(8);
    work_group_size = u32vec2_t(32);

    create_buffers();

    current_frame = 0;
    push_constants.current_frame = 0;
    push_constants.render_distance = static_cast<float>(hyper::rho);
    push_constants.window_size = window->get_size();

    set_main_camera(test_camera);

    fragment_shader_code = resources::load_file("../src/render/shader/shader.frag");
    vertex_shader_code   = resources::load_file("../src/render/shader/shader.vert");

    sphere = std::make_shared<substance_t>(
        std::make_shared<primitive::sphere_t<3>>(vec3_t(3.6, 0.78, 1.23), 2.3)
    );

    plane  = std::make_shared<substance_t>(
        std::make_shared<primitive::plane_t<3>>(vec3_t(0.0, 1.0, 0.0), 0)
    );

    substances.push_back(sphere);
    substances.push_back(plane);

    if (!init()){
        throw std::runtime_error("Error: Failed to initialise renderer subsystem.");
    }
}

void
renderer_t::cleanup_swapchain(){
    for (auto framebuffer : framebuffers){
	    vkDestroyFramebuffer(device->get_device(), framebuffer, nullptr);
    }

    command_buffers.clear();

    vkDestroyPipeline(device->get_device(), graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(device->get_device(), pipeline_layout, nullptr);
    vkDestroyRenderPass(device->get_device(), render_pass, nullptr);

    swapchain.reset(nullptr);
}

renderer_t::~renderer_t(){
    vkDestroyDescriptorSetLayout(device->get_device(), descriptor_layout, nullptr);

    cleanup_swapchain();

    vkDestroyPipeline(device->get_device(), compute_pipeline, nullptr);
    vkDestroyPipelineLayout(device->get_device(), compute_pipeline_layout, nullptr);

    for (int i = 0; i < frames_in_flight; i++){
        vkDestroySemaphore(device->get_device(), image_available_semas[i], nullptr);
        vkDestroySemaphore(device->get_device(), compute_done_semas[i], nullptr);
        vkDestroySemaphore(device->get_device(), render_finished_semas[i], nullptr);
        vkDestroyFence(device->get_device(), in_flight_fences[i], nullptr);
    }
}
  
bool 
renderer_t::create_compute_pipeline(){
    VkPushConstantRange push_const_range = {};
    push_const_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    push_const_range.size = sizeof(push_constant_t);
    push_const_range.offset = 0;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_layout;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;

    if (vkCreatePipelineLayout(
	    device->get_device(), &pipeline_layout_info, nullptr, &compute_pipeline_layout) != VK_SUCCESS
    ){
        std::cout << "Error: Failed to create pipeline layout." << std::endl;
	    return false;
    }

    std::string compute_shader_code = resources::load_file("../src/render/shader/shader.comp");

    VkShaderModule module = create_shader_module(compute_shader_code);

    VkComputePipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_create_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipeline_create_info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipeline_create_info.stage.module = module;
    pipeline_create_info.stage.pName = "main";
    pipeline_create_info.layout = compute_pipeline_layout;

    if (vkCreateComputePipelines(device->get_device(), VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &compute_pipeline) != VK_SUCCESS){
        return false;
    }

    vkDestroyShaderModule(device->get_device(), module, nullptr);    

    return true;
}

bool
renderer_t::init(){
    vkGetDeviceQueue(device->get_device(), device->get_present_family(), 0, &present_queue);

    swapchain = std::make_unique<swapchain_t>(device, push_constants.window_size, surface);

    if (!create_render_pass()){
        return false;
    }

    if (!create_descriptor_set_layout()){
        return false;
    }
    
    if (!create_graphics_pipeline()){
        return false;
    }
    
    if (!create_compute_pipeline()){
        return false;
    }

    graphics_command_pool = std::make_unique<command_pool_t>(device->get_device(), device->get_graphics_family());
    compute_command_pool = std::make_unique<command_pool_t>(device->get_device(), device->get_compute_family());

    if (!create_framebuffers()){
        return false;
    }

    if (!create_descriptor_pool()){
        return false;
    }

    if (!create_sync()){
        return false;
    }

    u32vec2_t image_size = work_group_count * work_group_size;
    render_texture = std::make_unique<texture_t>(
        10, allocator, device, image_size, VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY
    );

    std::vector<VkWriteDescriptorSet> write_desc_sets;
    for (auto descriptor_set : desc_sets){
        write_desc_sets.push_back(render_texture->get_descriptor_write(descriptor_set));
        for (auto buffer : buffers){
            write_desc_sets.push_back(buffer->get_write_descriptor_set(descriptor_set));
        }
    }

    vkUpdateDescriptorSets(device->get_device(), write_desc_sets.size(), write_desc_sets.data(), 0, nullptr);

    initialise_buffers();
 
    if (!create_command_buffers()){
        return false;
    }

    return true;
}

void
renderer_t::recreate_swapchain(){
    vkDeviceWaitIdle(device->get_device());
  
    cleanup_swapchain();    
    swapchain = std::make_unique<swapchain_t>(device, push_constants.window_size, surface);
    
    create_render_pass();
    create_graphics_pipeline();
    create_framebuffers();

    if (!create_command_buffers()){
        throw std::runtime_error("Error: failed to re-create command buffers on swapchain invalidation.");
    }
}

bool
renderer_t::create_render_pass(){
    VkAttachmentDescription colour_attachment = {};
    colour_attachment.format = swapchain->get_image_format();
    colour_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colour_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colour_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colour_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colour_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colour_attachment_ref = {};
    colour_attachment_ref.attachment = 0;
    colour_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass    = {};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colour_attachment_ref;
    subpass.pDepthStencilAttachment = nullptr;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
                             | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription> attachments = { colour_attachment };

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = attachments.size();
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    return vkCreateRenderPass(device->get_device(), &render_pass_info, nullptr, &render_pass) == VK_SUCCESS;
}

bool 
renderer_t::create_graphics_pipeline(){
    VkShaderModule vert_shader_module = create_shader_module(vertex_shader_code);
    VkShaderModule frag_shader_module = create_shader_module(fragment_shader_code);

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

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
        vert_create_info,
        frag_create_info
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkExtent2D extents = swapchain->get_extents();

    VkViewport viewport = {};
    viewport.x          = 0;
    viewport.y          = 0;
    viewport.width      = static_cast<float>(extents.width);
    viewport.height     = static_cast<float>(extents.height);
    viewport.minDepth   = 0;
    viewport.maxDepth   = 1;

    VkRect2D scissor = {};
    scissor.offset   = { 0, 0 };
    scissor.extent   = extents;

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

    VkPushConstantRange push_const_range = {};
    push_const_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
    push_const_range.size = sizeof(push_constant_t);
    push_const_range.offset = 0;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_layout;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;

    if (vkCreatePipelineLayout(
	    device->get_device(), &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS
    ){
        std::cout << "Error: Failed to create pipeline layout." << std::endl;
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
    pipeline_info.stageCount = shader_stages.size();
    pipeline_info.pStages = shader_stages.data();
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

    VkResult result = vkCreateGraphicsPipelines(
	    device->get_device(), VK_NULL_HANDLE, 1, 
        &pipeline_info, nullptr, &graphics_pipeline
    );

    if (result != VK_SUCCESS){
        std::cout << "Error: Failed to create graphics pipeline." << std::endl;
	    return false;
    }

    vkDestroyShaderModule(device->get_device(), vert_shader_module, nullptr);
    vkDestroyShaderModule(device->get_device(), frag_shader_module, nullptr);

    return true;
}

bool
renderer_t::create_framebuffers(){
    VkExtent2D extents = swapchain->get_extents();

    framebuffers.resize(swapchain->get_size());

    for (uint32_t i = 0; i < swapchain->get_size(); i++){
        VkImageView image_view = swapchain->get_image_view(i);

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = &image_view;
        framebuffer_info.width = extents.width;
        framebuffer_info.height = extents.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(
            device->get_device(), &framebuffer_info, nullptr, &framebuffers[i]) != VK_SUCCESS
        ){
            return false;
        }
    }

    return true;
}

bool
renderer_t::create_command_buffers(){
    command_buffers.clear();

    for (uint32_t i = 0; i < swapchain->get_size(); i++){
        command_buffers.push_back(graphics_command_pool->create_command_buffer(
            VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, [&](VkCommandBuffer command_buffer){
                VkRenderPassBeginInfo render_pass_info = {};
                render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                render_pass_info.renderPass = render_pass;
                render_pass_info.framebuffer = framebuffers[i];
                render_pass_info.renderArea.offset = { 0, 0 };
                render_pass_info.renderArea.extent = swapchain->get_extents();

                vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
                    vkCmdBindPipeline(
                        command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline
                    );

                    vkCmdBindDescriptorSets(
                        command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout,
                        0, 1, &desc_sets[i], 0, nullptr
                    );

                    vkCmdDraw(command_buffer, 3, 1, 0, 0);
                vkCmdEndRenderPass(command_buffer);
            }
        ));
    }

    return true;
}

bool 
renderer_t::create_descriptor_pool(){
    std::vector<VkDescriptorPoolSize> pool_sizes = {
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, swapchain->get_size() },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  swapchain->get_size() }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = pool_sizes.size();
    pool_info.pPoolSizes    = pool_sizes.data();
    pool_info.maxSets       = swapchain->get_size();

    if (vkCreateDescriptorPool(device->get_device(), &pool_info, nullptr, &desc_pool) != VK_SUCCESS){
	    return false;
    }

    std::vector<VkDescriptorSetLayout> layouts(swapchain->get_size(), descriptor_layout);

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool     = desc_pool;
    alloc_info.descriptorSetCount = swapchain->get_size();
    alloc_info.pSetLayouts        = layouts.data();

    desc_sets.resize(swapchain->get_size());
    if (vkAllocateDescriptorSets(device->get_device(), &alloc_info, desc_sets.data()) != VK_SUCCESS){
	    return false;
    }

    return true;
}

bool
renderer_t::create_descriptor_set_layout(){
    VkDescriptorSetLayoutBinding image_layout = {};
    image_layout.binding = 10;
    image_layout.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    image_layout.descriptorCount = 1;
    image_layout.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

    std::vector<VkDescriptorSetLayoutBinding> layouts = { image_layout };
    for (auto buffer : buffers){
        layouts.push_back(buffer->get_descriptor_set_layout_binding());
    }

    VkDescriptorSetLayoutCreateInfo layout_info = {};
    layout_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = layouts.size();
    layout_info.pBindings    = layouts.data();

    if (vkCreateDescriptorSetLayout(device->get_device(), &layout_info, nullptr, &descriptor_layout) != VK_SUCCESS){
        return false;
    }
 
    return true;
}

bool
renderer_t::create_sync(){
    image_available_semas.resize(frames_in_flight);
    compute_done_semas.resize(frames_in_flight);
    render_finished_semas.resize(frames_in_flight);
    in_flight_fences.resize(frames_in_flight);

    VkSemaphoreCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
   
    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
 
    for (int i = 0; i < frames_in_flight; i++){
        if (vkCreateSemaphore(
            device->get_device(), &create_info, nullptr, &image_available_semas[i]) != VK_SUCCESS
        ){
            return false;
        }

        if (vkCreateSemaphore(
            device->get_device(), &create_info, nullptr, &render_finished_semas[i]) != VK_SUCCESS
        ){
            return false;
        }

        if (vkCreateSemaphore(
            device->get_device(), &create_info, nullptr, &compute_done_semas[i]) != VK_SUCCESS
        ){
            return false;
        }

        if (vkCreateFence(device->get_device(), &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS){
            return false;
        }
    }

    return true;
}

void 
renderer_t::present(uint32_t image_index) const {
    VkSwapchainKHR swapchain_handle = swapchain->get_handle();
    VkPresentInfoKHR present_info   = {};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = &render_finished_semas[current_frame];
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = &swapchain_handle;
    present_info.pImageIndices      = &image_index;
    present_info.pResults           = nullptr;
    
    vkQueuePresentKHR(present_queue, &present_info);
}

void
renderer_t::render(){
    push_constants.current_frame++;

    handle_requests();

    if (auto camera = main_camera.lock()){
        push_constants.camera_position = camera->get_position().cast<float>();
        push_constants.camera_right = camera->get_right().cast<float>();
        push_constants.camera_up = camera->get_up().cast<float>();
    }
   
    uint32_t image_index;
    vkAcquireNextImageKHR(
        device->get_device(), swapchain->get_handle(), ~((uint64_t) 0), image_available_semas[current_frame], 
        VK_NULL_HANDLE, &image_index
    );

    auto compute_command_buffer = compute_command_pool->create_command_buffer(
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, [&](VkCommandBuffer command_buffer){
            vkCmdPushConstants(
                command_buffer, compute_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT,
                0, sizeof(push_constant_t), &push_constants
            );

            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline);
            vkCmdBindDescriptorSets(
                command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout,
                0, 1, &desc_sets[image_index], 0, nullptr
            );
            vkCmdDispatch(command_buffer, work_group_count[0], work_group_count[1], 1);
        }
    );

    compute_command_buffer->submit(
        image_available_semas[current_frame], compute_done_semas[current_frame], 
        in_flight_fences[current_frame], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
    );
    
    command_buffers[image_index]->submit(
        compute_done_semas[current_frame], render_finished_semas[current_frame], 
        in_flight_fences[current_frame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    );

    present(image_index);

    current_frame = (current_frame + 1) % frames_in_flight; 
    vkWaitForFences(device->get_device(), 1, &in_flight_fences[current_frame], VK_TRUE, ~((uint64_t) 0));
    vkResetFences(device->get_device(), 1, &in_flight_fences[current_frame]);    
}

VkShaderModule
renderer_t::create_shader_module(std::string code){
    const char * c_string = code.c_str();
    
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO; 
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t *>(c_string);

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device->get_device(), &create_info, nullptr, &shader_module) != VK_SUCCESS){
	    throw std::runtime_error("Error: Failed to create shader module.");
    }
    return shader_module;
}

void 
renderer_t::set_main_camera(std::weak_ptr<camera_t> camera){
    main_camera = camera;
}

void 
renderer_t::handle_requests(){
    static std::vector<request_t> blank_request(1);

    vkDeviceWaitIdle(device->get_device()); //TODO: remove this by baking in buffer updates

    request_buffer->read(requests, 0);

    for (uint32_t i = 0; i < work_group_count[0] * work_group_count[1]; i++){
        request_t r = requests[i];

        if (r.child != 0){
            std::vector<octree_node_t> new_node(8);
            if (auto substance = substances[r.objectID].lock()){
                new_node = octree_node_t::create(r.aabb, substance->get_sdf());
            }

            octree_buffer->write(new_node, r.child);
            request_buffer->write(blank_request, i);
        }
    }   
}

void 
renderer_t::create_buffers(){
    uint32_t count = work_group_count[0] * work_group_count[1];
    uint32_t size = work_group_size[0] * work_group_size[1];

    octree_buffer = std::make_shared<buffer_t>(
        allocator, 1, device, sizeof(octree_node_t) * count * size, VMA_MEMORY_USAGE_CPU_TO_GPU
    );

    substance_buffer = std::make_shared<buffer_t>(
        allocator, 2, device, sizeof(substance_t::data_t) * size, VMA_MEMORY_USAGE_CPU_TO_GPU
    );

    requests.resize(count);
    request_buffer = std::make_shared<buffer_t>(
        allocator, 3, device, sizeof(request_t) * count, VMA_MEMORY_USAGE_GPU_TO_CPU
    );

    buffers = { octree_buffer, substance_buffer, request_buffer };

    // add persistent GPU state buffer
    buffers.push_back(std::make_shared<buffer_t>(
        allocator, 4, device, count * size * 32, VMA_MEMORY_USAGE_GPU_ONLY
    ));
}

void
renderer_t::initialise_buffers(){
    std::vector<substance_t::data_t> substance_data(work_group_size[0] * work_group_size[1]);
    substance_data[0].root = 0;
    substance_data[1].root = 8;
    substance_buffer->write(substance_data, 0);

    f32vec4_t bounds(-hyper::rho, -hyper::rho, -hyper::rho, 2 * hyper::rho);

    std::vector<octree_node_t> initial_octree;
    initial_octree.resize(work_group_count[0] * work_group_count[1] * work_group_size[0] * work_group_size[1]);

    for (uint32_t j = 0; j < substances.size(); j++){
        if (auto substance = substances[j].lock()){
            std::vector<octree_node_t> root_node = octree_node_t::create(bounds, substance->get_sdf());

            for (uint32_t i = 0; i < initial_octree.size(); i += work_group_size[0] * work_group_size[1]){
                for (uint32_t k = 0; k < 8; k++){
                    initial_octree[i + j * 8 + k] = root_node[k];
                }
            }
        }
    }
    
    octree_buffer->write(initial_octree, 0);
}