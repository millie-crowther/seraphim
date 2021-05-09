//
// Created by millie on 09/05/2021.
//

#ifndef SERAPHIM_SHADER_H
#define SERAPHIM_SHADER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/device.h"

typedef struct shader_t {
    char * code;
    VkShaderModule module;
    device_t * device;
    VkShaderStageFlagBits stage;
} shader_t;

bool shader_create(shader_t *shader, const char *filename, device_t *device, VkShaderStageFlagBits stage);
void shader_destroy(shader_t *shader);

void shader_pipeline_stage_create_info(shader_t * shader, VkPipelineShaderStageCreateInfo * create_info);

#endif //SERAPHIM_SHADER_H
