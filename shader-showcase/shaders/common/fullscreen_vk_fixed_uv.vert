#version 460
// DIAG: Fixed vUV vertex shader - outputs constant (0.5, 0.5) for all vertices
layout(location=0) out vec2 vUV;
void main() {
    float x = float(gl_VertexIndex == 1 ? 3 : -1);
    float y = float(gl_VertexIndex == 2 ? 3 : -1);
    gl_Position = vec4(x, y, 0, 1);
    vUV = vec2(0.5, 0.5);
}