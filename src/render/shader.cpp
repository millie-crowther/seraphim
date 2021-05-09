//
// Created by millie on 09/05/2021.
//
#include <cstring>
#include <ui/file.h>
#include <cstdlib>
#include "render/shader.h"

bool shader_create(shader_t *shader, const char *filename, device_t *device) {
    shader->code = file_load_text(filename);
    shader->device = device;

    if (shader->code == NULL){
        return false;
    }

    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = strlen(shader->code);
    create_info.pCode = (const uint32_t *) shader->code;

    return vkCreateShaderModule(device->device, &create_info, NULL, &shader->module) ==
        VK_SUCCESS;
}

void shader_destroy(shader_t *shader) {
    free(shader->code);
    shader->code = NULL;
    vkDestroyShaderModule(shader->device->device, shader->module, NULL);
}
