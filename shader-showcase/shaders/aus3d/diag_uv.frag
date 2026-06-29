#version 460
// Diagnostic: verify UV coordinates
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
void main() {
    // top-left=red, top-right=green, bottom-left=blue, bottom-right=yellow
    outColor = vec4(vUV.x, vUV.y, 1.0 - vUV.x, 1.0);
}
