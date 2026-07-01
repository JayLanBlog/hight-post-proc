#version 460
// LiquidGlass: 背景纹理直通 — 渲染背景到RT

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D uInputTex;

void main() {
    outColor = texture(uInputTex, vUV);
}