#version 450

out gl_PerVertex {
    vec4 gl_Position;
};

const vec2 Father     = vec2(-3, -1);
const vec2 Son        = vec2( 0,  3);
const vec2 HolySpirit = vec2( 3, -1);

const vec2 Trinity[3] = { Father, Son, HolySpirit };

void main(){
    gl_Position = vec4(Trinity[gl_VertexID], 0, 1);
}