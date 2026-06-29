#version 460
// aus3d_shared.vert — fullscreen triangle (no vertex input)
layout(location=0) out vec2 vUV;
void main() {
    vec2 pos[3] = vec2[](vec2(-1,-1), vec2(3,-1), vec2(-1,3));
    gl_Position = vec4(pos[gl_VertexIndex], 0, 1);
    vUV = pos[gl_VertexIndex] * 0.5 + 0.5;
}
