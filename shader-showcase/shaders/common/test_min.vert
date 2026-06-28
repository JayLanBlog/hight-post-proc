#version 460
// Minimal test: only position output, no vertex input
layout(location=0) out vec2 vUV;
void main() {
    // Hardcoded triangle (no vertex buffer needed)
    vec2 positions[3] = vec2[](
        vec2(-1,-1), vec2(3,-1), vec2(-1,3)
    );
    gl_Position = vec4(positions[gl_VertexIndex], 0, 1);
    vUV = positions[gl_VertexIndex] * 0.5 + 0.5;
}
