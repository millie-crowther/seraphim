#version 450

out gl_PerVertex {
    vec4 gl_Position;
};

void main(){
    gl_Position = vec4(vec2(gl_VertexID & 2, (gl_VertexID << 1) & 2) * 2 - 1, 0, 1);
}
