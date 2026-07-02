// shaders/vfx_fire/dissolve.frag
// 溶解后处理Shader - 复刻Unity VFX Graph Magic Fire Book的溶解效果
// 算法: Remap(0.5-1.3) + Noise + OneMinus + Step(0.29) + AlphaClip + EdgeGlow
// UBO布局: 与VulkanBackend 224字节布局对齐 (P0-P5 → uParams)
#version 450

layout(binding = 0) uniform sampler2D uInputTex;   // 书本渲染结果RT
layout(binding = 1, std140) uniform Params {
    float uP0; float uP1; float uP2; float uP3;     // offset 0:  dissolveAmount, edgeWidth, noiseScaleX, noiseScaleY
    float uP4; float uP5; float uP6; float uP7;     // offset 16: noiseOffsetX, noiseOffsetY, (unused), (unused)
    float uTime; float uFrameCount; float uPad0; float uPad1; // offset 32
    mat4 uMVP;         // offset 48
    mat4 uModelView;   // offset 112
    vec4 uLightDir;    // offset 176
    vec4 uLightColor;  // offset 192
    vec4 uEyePos;      // offset 208
    vec4 uParams;      // offset 224 (unused, kept for alignment)
};
layout(binding = 2) uniform sampler2D uAuxTex;  // 噪声纹理

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

void main() {
    vec4 bookColor = texture(uInputTex, vUV);

    // Remap: dissolveAmount 0-1 -> 0.5-1.3
    float remapped = 0.5 + uP0 * (1.3 - 0.5);

    // Sample noise (with scale and animated offset)
    vec2 noiseUV = vUV * vec2(uP2, uP3) + vec2(uP4, uP5);
    float noise = texture(uAuxTex, noiseUV).r;

    // OneMinus (溶解区域 = 1 - noise clamped)
    float dissolve = 1.0 - clamp(remapped + noise * 0.3, 0.0, 1.0);

    // Step threshold 0.29 (来自ShaderGraph)
    float clip = step(0.29, dissolve);

    // Alpha clip
    if (clip < 0.5) discard;

    // Edge detection: smoothstep around dissolve threshold
    float edgeLow = remapped - uP1;
    float edgeHigh = remapped + uP1;
    float edgeMask = 1.0 - smoothstep(edgeLow, edgeHigh, dissolve);

    // Edge color: 高亮度橙色 (6.5, 0.894, 0.0)
    vec3 edgeColor = vec3(6.5, 0.894, 0.0) * edgeMask;

    outColor = vec4(bookColor.rgb + edgeColor, 1.0);
}