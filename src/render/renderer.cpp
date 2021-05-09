#include "render/renderer.h"

#include "core/constant.h"
#include "render/texture.h"

#include <chrono>
#include <stdexcept>
#include <ui/file.h>

using namespace srph;

renderer_t::renderer_t(device_t *device, substance_t *substances, uint32_t *num_substances, VkSurfaceKHR surface,
                       srph::window_t *window, std::shared_ptr<srph::camera_t> test_camera,
                       srph::u32vec2_t work_group_count,
                       srph::u32vec2_t work_group_size, uint32_t max_image_size, material_t *materials,
                       uint32_t *num_materials,
                       sdf_t *sdfs, uint32_t *num_sdfs) {
    this->device = device;
    this->surface = surface;
    this->work_group_count = work_group_count;
    this->work_group_size = work_group_size;
    this->substances = substances;
    this->num_substances = num_substances;

    texture_size = max_image_size / patch_sample_size;

    start = std::chrono::high_resolution_clock::now();

    current_frame = 0;
    push_constants.current_frame = 0;
    push_constants.render_distance = (float) rho;
    push_constants.window_size = window->get_size();
    push_constants.phi_initial = 0;
    push_constants.focal_depth = 1.0;
    push_constants.number_of_requests = number_of_requests;
    push_constants.texture_size = texture_size;
    push_constants.texture_depth =
            texture_pool_size / texture_size / texture_size + 1;
    push_constants.geometry_pool_size = geometry_pool_size;
    push_constants.texture_pool_size = texture_pool_size;
    push_constants.epsilon = (float) epsilon;

    set_main_camera(test_camera);

    vkGetDeviceQueue(device->device, device->present_family, 0, &present_queue);

    swapchain =
        std::make_unique<swapchain_t>(device, push_constants.window_size, surface);

    create_render_pass();

    create_buffers();
    request_handler_create(&request_handler, texture_size, push_constants.texture_depth, patch_sample_size, sdfs,
                           num_sdfs, materials, num_materials, device);

    create_descriptor_set_layout();
    create_graphics_pipeline();
    create_compute_pipeline();

    graphics_command_pool =
        std::make_unique<command_pool_t>(device->device, device->graphics_family);
    compute_command_pool =
        std::make_unique<command_pool_t>(device->device, device->compute_family);

    create_framebuffers();
    create_descriptor_pool();
    create_sync();

    render_texture = std::make_unique<texture_t>(
        10, device,
        u32vec3_t(work_group_count[0] * work_group_size[0],
                  work_group_count[1] * work_group_size[1], 1u),
        VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    std::vector<VkWriteDescriptorSet> write_desc_sets;
    for (auto descriptor_set : desc_sets) {
        write_desc_sets.push_back(
            render_texture->get_descriptor_write(descriptor_set));
        write_desc_sets.push_back(
            request_handler.normal_texture->get_descriptor_write(descriptor_set));
        write_desc_sets.push_back(
                request_handler.colour_texture->get_descriptor_write(descriptor_set));

        write_desc_sets.push_back(
                request_handler.patch_buffer.get_write_descriptor_set(descriptor_set));
        write_desc_sets.push_back(
            substance_buffer.get_write_descriptor_set(descriptor_set));
        write_desc_sets.push_back(
                request_handler.request_buffer.get_write_descriptor_set(descriptor_set));
        write_desc_sets.push_back(
            light_buffer.get_write_descriptor_set(descriptor_set));
        write_desc_sets.push_back(
            pointer_buffer.get_write_descriptor_set(descriptor_set));
        write_desc_sets.push_back(
            frustum_buffer.get_write_descriptor_set(descriptor_set));
        write_desc_sets.push_back(
            lighting_buffer.get_write_descriptor_set(descriptor_set));
        write_desc_sets.push_back(
                request_handler.texture_hash_buffer.get_write_descriptor_set(descriptor_set));
    }

    vkUpdateDescriptorSets(device->device, write_desc_sets.size(),
                           write_desc_sets.data(), 0, NULL);

    create_command_buffers();
}

void renderer_t::cleanup_swapchain() {
    for (auto framebuffer : framebuffers) {
        vkDestroyFramebuffer(device->device, framebuffer, NULL);
    }

    command_buffers.clear();

    vkDestroyPipeline(device->device, graphics_pipeline, NULL);
    vkDestroyPipelineLayout(device->device, pipeline_layout, NULL);
    vkDestroyRenderPass(device->device, render_pass, NULL);

    swapchain.reset(NULL);
}

renderer_t::~renderer_t() {
    shader_destroy(&vertex_shader);
    shader_destroy(&fragment_shader);

    vkDestroyDescriptorSetLayout(device->device, descriptor_layout, NULL);

    cleanup_swapchain();

    vkDestroyPipeline(device->device, compute_pipeline, NULL);
    vkDestroyPipelineLayout(device->device, compute_pipeline_layout, NULL);

    for (int i = 0; i < frames_in_flight; i++) {
        vkDestroySemaphore(device->device, image_available_semas[i], NULL);
        vkDestroySemaphore(device->device, compute_done_semas[i], NULL);
        vkDestroySemaphore(device->device, render_finished_semas[i], NULL);
        vkDestroyFence(device->device, in_flight_fences[i], NULL);
    }

    request_handler_destroy(&request_handler);

    buffer_destroy(&substance_buffer);
    buffer_destroy(&light_buffer);
    buffer_destroy(&pointer_buffer);
    buffer_destroy(&frustum_buffer);
    buffer_destroy(&lighting_buffer);
}

void renderer_t::create_compute_pipeline() {
    VkPushConstantRange push_const_range = {};
    push_const_range.stageFlags =
        VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    push_const_range.size = sizeof(push_constant_t);
    push_const_range.offset = 0;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_layout;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;

    if (vkCreatePipelineLayout(device->device, &pipeline_layout_info, NULL,
                               &compute_pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error("Error: Failed to create compute pipeline layout.");
    }

    shader_t compute_shader;
    if (!shader_create(&compute_shader, "../src/render/shader/comp.glsl", device, VK_SHADER_STAGE_COMPUTE_BIT)){
        printf("Error: Failed to create compute shader");
        exit(1);
    }

    VkComputePipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_create_info.layout = compute_pipeline_layout;

    shader_pipeline_stage_create_info(&compute_shader, &pipeline_create_info.stage);

    if (vkCreateComputePipelines(device->device, VK_NULL_HANDLE, 1,
                                 &pipeline_create_info, NULL,
                                 &compute_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("Error: Failed to create compute pipeline.");
    }

    shader_destroy(&compute_shader);
}

void renderer_t::recreate_swapchain() {
    vkDeviceWaitIdle(device->device);

    cleanup_swapchain();
    swapchain =
        std::make_unique<swapchain_t>(device, push_constants.window_size, surface);

    create_render_pass();
    create_graphics_pipeline();
    create_framebuffers();
    create_command_buffers();
}

void renderer_t::create_render_pass() {
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

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colour_attachment_ref;
    subpass.pDepthStencilAttachment = NULL;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription> attachments = {colour_attachment};

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = attachments.size();
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if (vkCreateRenderPass(device->device, &render_pass_info, NULL, &render_pass) !=
        VK_SUCCESS) {
        throw std::runtime_error("Error: Failed to create render pass.");
    }
}

void renderer_t::create_graphics_pipeline() {
    if (!shader_create(&vertex_shader, "../src/render/shader/vert.glsl", device, VK_SHADER_STAGE_VERTEX_BIT)){
        printf("Error: Failed to create vertex shader");
        exit(1);
    }

    if (!shader_create(&fragment_shader, "../src/render/shader/frag.glsl", device, VK_SHADER_STAGE_FRAGMENT_BIT)){
        printf("Error: Failed to create fragment shader");
        exit(1);
    }

    VkPipelineShaderStageCreateInfo shader_stages[2];
    shader_pipeline_stage_create_info(&vertex_shader, &shader_stages[0]);
    shader_pipeline_stage_create_info(&fragment_shader, &shader_stages[1]);

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkExtent2D extents = swapchain->get_extents();

    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = static_cast<float>(extents.width);
    viewport.height = static_cast<float>(extents.height);
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = extents;

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
    multisample_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_info.sampleShadingEnable = VK_FALSE;
    multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_info.minSampleShading = 1.0f;
    multisample_info.pSampleMask = NULL;
    multisample_info.alphaToCoverageEnable = VK_FALSE;
    multisample_info.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colour_blending = {};
    colour_blending.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colour_blending.blendEnable = VK_FALSE;
    colour_blending.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colour_blending.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colour_blending.colorBlendOp = VK_BLEND_OP_ADD;
    colour_blending.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colour_blending.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colour_blending.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colour_blend_info = {};
    colour_blend_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
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
    push_const_range.stageFlags =
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
    push_const_range.size = sizeof(push_constant_t);
    push_const_range.offset = 0;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_layout;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;

    if (vkCreatePipelineLayout(device->device, &pipeline_layout_info, NULL,
                               &pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error(
            "Error: Failed to create graphics pipeline layout.");
    }

    VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_FALSE;
    depth_stencil.depthWriteEnable = VK_FALSE;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

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
    pipeline_info.pDynamicState = NULL;
    pipeline_info.layout = pipeline_layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(device->device, VK_NULL_HANDLE, 1, &pipeline_info,
                                  NULL, &graphics_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("Error: Failed to create graphics pipeline.");
    }
}

void renderer_t::create_framebuffers() {
    VkExtent2D extents = swapchain->get_extents();
    framebuffers.resize(swapchain->get_size());

    for (uint32_t i = 0; i < swapchain->get_size(); i++) {
        VkImageView image_view = swapchain->get_image_view(i);

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = &image_view;
        framebuffer_info.width = extents.width;
        framebuffer_info.height = extents.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(device->device, &framebuffer_info, NULL,
                                &framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error: Failed to create framebuffer.");
        }
    }
}

void renderer_t::create_command_buffers() {
    command_buffers.clear();

    for (uint32_t i = 0; i < swapchain->get_size(); i++) {
        command_buffers.push_back(
            graphics_command_pool->reusable_buffer([&](auto command_buffer) {
                VkRenderPassBeginInfo render_pass_info = {};
                render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                render_pass_info.renderPass = render_pass;
                render_pass_info.framebuffer = framebuffers[i];
                render_pass_info.renderArea.offset = {0, 0};
                render_pass_info.renderArea.extent = swapchain->get_extents();
                vkCmdBeginRenderPass(command_buffer, &render_pass_info,
                                     VK_SUBPASS_CONTENTS_INLINE);
                vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  graphics_pipeline);
                vkCmdBindDescriptorSets(
                    command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout,
                    0, 1, &desc_sets[i], 0, NULL);
                vkCmdDraw(command_buffer, 3, 1, 0, 0);
                vkCmdEndRenderPass(command_buffer);
            }));
    }
}

void renderer_t::create_descriptor_pool() {
    std::vector<VkDescriptorPoolSize> pool_sizes = {
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, swapchain->get_size()},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, swapchain->get_size()},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, swapchain->get_size()}};

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = pool_sizes.size();
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = swapchain->get_size();

    if (vkCreateDescriptorPool(device->device, &pool_info, NULL, &desc_pool) !=
        VK_SUCCESS) {
        throw std::runtime_error("Error: Failed to create descriptor pool.");
    }

    std::vector<VkDescriptorSetLayout> layouts(swapchain->get_size(),
                                               descriptor_layout);

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = desc_pool;
    alloc_info.descriptorSetCount = swapchain->get_size();
    alloc_info.pSetLayouts = layouts.data();

    desc_sets.resize(swapchain->get_size());
    if (vkAllocateDescriptorSets(device->device, &alloc_info, desc_sets.data()) !=
        VK_SUCCESS) {
        throw std::runtime_error("Error: Failed to allocate descriptor sets.");
    }
}

void renderer_t::create_descriptor_set_layout() {
    VkDescriptorSetLayoutBinding image_layout = {};
    image_layout.binding = 10;
    image_layout.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    image_layout.descriptorCount = 1;
    image_layout.stageFlags =
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

    std::vector<VkDescriptorSetLayoutBinding> layouts = {
            image_layout,
            request_handler.normal_texture->get_descriptor_layout_binding(),
            request_handler.colour_texture->get_descriptor_layout_binding(),

            request_handler.patch_buffer.get_descriptor_set_layout_binding(),
            substance_buffer.get_descriptor_set_layout_binding(),
            light_buffer.get_descriptor_set_layout_binding(),
            request_handler.request_buffer.get_descriptor_set_layout_binding(),
            pointer_buffer.get_descriptor_set_layout_binding(),
            frustum_buffer.get_descriptor_set_layout_binding(),
            lighting_buffer.get_descriptor_set_layout_binding(),
            request_handler.texture_hash_buffer.get_descriptor_set_layout_binding(),
    };

    VkDescriptorSetLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = layouts.size();
    layout_info.pBindings = layouts.data();

    if (vkCreateDescriptorSetLayout(device->device, &layout_info, NULL,
                                    &descriptor_layout) != VK_SUCCESS) {
        throw std::runtime_error("Error: Failed to create descriptor set layout.");
    }
}

void renderer_t::create_sync() {
    VkSemaphoreCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    uint32_t result = VK_SUCCESS;
    for (int i = 0; i < frames_in_flight; i++) {
        result |= vkCreateSemaphore(device->device, &create_info, NULL,
                                    &image_available_semas[i]);
        result |= vkCreateSemaphore(device->device, &create_info, NULL,
                                    &render_finished_semas[i]);
        result |= vkCreateSemaphore(device->device, &create_info, NULL,
                                    &compute_done_semas[i]);
        result |=
            vkCreateFence(device->device, &fence_info, NULL, &in_flight_fences[i]);
    }

    if (result != VK_SUCCESS) {
        throw std::runtime_error(
            "Error: Failed to create synchronisation primitives.");
    }
}

void renderer_t::present(uint32_t image_index) const {
    VkSwapchainKHR swapchain_handle = swapchain->get_handle();
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_finished_semas[current_frame];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain_handle;
    present_info.pImageIndices = &image_index;
    present_info.pResults = NULL;

    vkQueuePresentKHR(present_queue, &present_info);
}

void renderer_t::render() {
    frames++;

    uint32_t size = work_group_size[0] * work_group_size[1];

    // write substances
    std::vector<data_t> substance_data;
    for (size_t i = 0; i < *num_substances; i++) {
        substance_t *s = &substances[i];
        substance_data.push_back(
            s->get_data(&main_camera.lock()->transform.position));
    }
    substance_data.resize(size);

    std::sort(substance_data.begin(), substance_data.end(), data_t::comparator_t());

    substance_buffer.write(substance_data.data(), substance_data.size(), 0);

    // write lights
    std::vector<light_t> lights(size);
    lights[0] = light_t(f32vec3_t(0, 4.0f, -4.0f), f32vec4_t(50.0f));
    light_buffer.write(lights.data(), lights.size(), 0);

    if (auto camera = main_camera.lock()) {
        srph_camera_transformation_matrix(camera.get(),
                                          push_constants.eye_transform);
    }

    uint32_t image_index;
    vkAcquireNextImageKHR(
        device->device, swapchain->get_handle(), ~static_cast<uint64_t>(0),
        image_available_semas[current_frame], VK_NULL_HANDLE, &image_index);

    request_handler_handle_requests(&request_handler);

    compute_command_pool
        ->one_time_buffer([&](auto command_buffer) {
            substance_buffer.record_write(command_buffer);
            light_buffer.record_write(command_buffer);

            request_handler_record_write(&request_handler, command_buffer);

            vkCmdPushConstants(command_buffer, compute_pipeline_layout,
                               VK_SHADER_STAGE_COMPUTE_BIT, 0,
                               sizeof(push_constant_t), &push_constants);
            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                              compute_pipeline);
            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                                    compute_pipeline_layout, 0, 1,
                                    &desc_sets[image_index], 0, NULL);
            vkCmdDispatch(command_buffer, work_group_count[0], work_group_count[1],
                          1);
        })
        ->submit(image_available_semas[current_frame],
                 compute_done_semas[current_frame], in_flight_fences[current_frame],
                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    command_buffers[image_index]->submit(
        compute_done_semas[current_frame], render_finished_semas[current_frame],
        in_flight_fences[current_frame],
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    present(image_index);

    vkWaitForFences(device->device, 1, &in_flight_fences[current_frame], VK_TRUE,
                    ~((uint64_t)0));
    vkResetFences(device->device, 1, &in_flight_fences[current_frame]);

    push_constants.current_frame++;
    current_frame = (current_frame + 1) % frames_in_flight;
}

void renderer_t::set_main_camera(std::weak_ptr<camera_t> camera) {
    main_camera = camera;
}


void renderer_t::create_buffers() {
    uint32_t c = work_group_count[0] * work_group_count[1];
    uint32_t s = work_group_size[0] * work_group_size[1];

    buffer_create(&light_buffer, 3, device, s, true, sizeof(light_t));
    buffer_create(&substance_buffer, 4, device, s, true, sizeof(data_t));
    buffer_create(&pointer_buffer, 5, device, c * s, true, sizeof(uint32_t));
    buffer_create(&frustum_buffer, 6, device, c, true, sizeof(float) * 2);
    buffer_create(&lighting_buffer, 7, device, c, true, sizeof(float) * 4);
}

int renderer_t::get_frame_count() {
    int f = frames;
    frames = 0;
    return f;
}
