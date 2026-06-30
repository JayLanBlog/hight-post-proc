#version 460
// Pass1: 7-tap垂直高斯模糊
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

const float weights[7] = {0.0205, 0.0855, 0.232, 0.324, 0.232, 0.0855, 0.0205};

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(uInputTex, 0));
    vec4 col = vec4(0.0);
    for (int i = 0; i < 7; i++) {
        float offset = float(i - 3) * P0 * 2.0;
        col += texture(uInputTex, vUV + vec2(0.0, offset) * texelSize) * weights[i];
    }
    outColor = col;
}