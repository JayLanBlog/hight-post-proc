#version 460
// Compute vUV directly from gl_VertexIndex (Vulkan-safe)
layout(location=0) out vec2 vUV;
void main() {
    float x = float(gl_VertexIndex == 1 ? 3 : -1);
    float y = float(gl_VertexIndex == 2 ? 3 : -1);
    gl_Position = vec4(x, y, 0, 1);
    vUV = vec2((x + 1.0) * 0.5, (y + 1.0) * 0.5);
}