#version 450

layout(location = 0) out vec4 out_colour;

layout (binding = 10, rgba8) uniform readonly image2D render_texture;

void main(){
    if (gl_FragCoord.x < 250 && gl_FragCoord.y < 250){
        out_colour = imageLoad(render_texture, ivec2(gl_FragCoord.xy));
    }
}