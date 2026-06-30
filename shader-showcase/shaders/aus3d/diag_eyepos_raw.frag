#version 460
// DIAG: Direct UBO read — output eyePos as color (no raycast)
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
void main() {
    // Normalize eyePos to [0,1] range (camera at distance ~3.0)
    outColor = vec4(uEyePos / 5.0, 1.0);
}