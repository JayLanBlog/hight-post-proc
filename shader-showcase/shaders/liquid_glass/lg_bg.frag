#version 460
layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D uInputTex;
layout(std140, binding = 1) uniform Params { float P0; float P1; float P2; float P3; float P4; float P5; vec2 uRes; float uTime; float uFC; mat4 m0; mat4 m1; vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2; };
void main() {
    vec2 ndc = (vUV - 0.5) * 2.0;
    if (abs(ndc.x) > P0 || abs(ndc.y) > P1) { outColor = vec4(0.1, 0.1, 0.1, 1.0); return; }
    vec2 texUV = (ndc / vec2(P0, P1) + 1.0) * 0.5;
    outColor = texture(uInputTex, texUV);
}