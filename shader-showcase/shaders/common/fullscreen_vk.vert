#version 450
// Vulkan-only fullscreen quad — uses gl_VertexIndex, no vertex input

layout(location = 0) out vec2 vUV;

void main() {
    // 2 triangles: indices 0,1,2 then 0,2,3 → covers NDC [-1,1]×[-1,1]
    // Triangle strip would need 4 vertices, use gl_VertexIndex to compute position
    float x = float((gl_VertexIndex & 1) << 2) - 1.0;  // -1, 3, -1, 3
    float y = float((gl_VertexIndex & 2) << 1) - 1.0;  // -1, -1, 3, 3
    vUV = vec2((x + 1.0) * 0.5, (y + 1.0) * 0.5);
    gl_Position = vec4(x, y, 0.0, 1.0);
}
