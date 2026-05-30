#version 460
layout(location=0) in vec2 aPos;
layout(location=0) out vec2 vUV;

void main() {
    vUV = (aPos + 1.0) * 0.5;
    vUV.y = 1.0 - vUV.y;  // Flip Y for correct display
    gl_Position = vec4(aPos, 0, 1);
}
