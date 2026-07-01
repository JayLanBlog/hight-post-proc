#version 460
// DIAG: 验证pass0→pass1数据传递
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

void main() {
    // 诊断: 直接输出pass0的数据
    vec4 pass0Data = texture(uInputTex, vUV);
    outColor = pass0Data;
}