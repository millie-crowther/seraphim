#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 out_colour;

layout( push_constant ) uniform window_block {
    uvec2 window_size;
} push_constant;

in vec4 gl_FragCoord;

void main() {
    if (length(gl_FragCoord.xy - push_constant.window_size / 2) < 100){
        out_colour = vec4(1, 0, 1, 1);
    } else {
        out_colour = vec4(0.5, 0.5, 0.75, 1);
    }
}
