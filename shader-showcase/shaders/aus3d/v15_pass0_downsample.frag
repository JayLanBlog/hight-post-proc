#version 460
// Pass0: 2x2降采样
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(uInputTex, 0));
    vec2 uv = vUV;
    vec4 c0 = texture(uInputTex, uv + vec2(-0.5, -0.5) * texelSize);
    vec4 c1 = texture(uInputTex, uv + vec2( 0.5, -0.5) * texelSize);
    vec4 c2 = texture(uInputTex, uv + vec2(-0.5,  0.5) * texelSize);
    vec4 c3 = texture(uInputTex, uv + vec2( 0.5,  0.5) * texelSize);
    outColor = (c0 + c1 + c2 + c3) * 0.25;
}