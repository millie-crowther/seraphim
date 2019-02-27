#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 out_colour;

in vec4 gl_FragCoord;

void main() {
    vec2 p = gl_FragCoord.xy;
    out_colour = vec4(1, 0, 0, 1);
}
