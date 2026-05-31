#version 460

// Vulkan: no vertex buffer, generate fullscreen triangle from VertexIndex
// Triangle vertices: (-1,-1), (3,-1), (-1,3) covers entire clip space
layout(location=0) out vec2 vUV;

void main() {
    // Generate fullscreen triangle from vertex index
    // idx 0 -> (-1, -1), idx 1 -> (3, -1), idx 2 -> (-1, 3)
    float x = float(gl_VertexIndex == 1 ? 3 : -1);
    float y = float(gl_VertexIndex == 2 ? 3 : -1);
    gl_Position = vec4(x, y, 0, 1);
    vUV = (gl_Position.xy + 1.0) * 0.5;
}
