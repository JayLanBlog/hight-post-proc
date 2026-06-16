#version 460

// For OpenGL: provide vertex input via VAO
layout(location=0) in vec2 aPos;
layout(location=0) out vec2 vUV;

void main() {
    // OpenGL path: use aPos from VAO
    vUV = (aPos + 1.0) * 0.5;
    gl_Position = vec4(aPos, 0, 1);
}
