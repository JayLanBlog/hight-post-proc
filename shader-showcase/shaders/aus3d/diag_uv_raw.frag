#version 460
// DIAG: Direct UV output — verify UV coordinates (no raycast)
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
void main() {
    outColor = vec4(vUV, 0.0, 1.0);
}