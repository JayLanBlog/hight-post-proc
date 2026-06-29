#version 460
// Diagnostic: verify UBO lightDir
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};
void main() {
    // Show lightDir as RGB (abs to handle negative)
    outColor = vec4(abs(uLightDir.x), abs(uLightDir.y), abs(uLightDir.z), 1.0);
}
