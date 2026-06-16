#version 460
layout(location=0) in vec2 vUV;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0;
    float uParamFloat1;
    float uParamFloat2;
    float uParamFloat3;
    float uParamFloat4;
    float uParamFloat5;
    vec2 uResolution;
    float uTime;
    float uFrameCount;
};

void main() {
    vec3 color = texture(uInputTex, vUV).rgb;

    // ---- 颜色量化（色阶映射） ----
    // 原公式 floor(color * N) / (N-1) 会导致最高阶永远达不到（overflow 后 clamp 回 1.0）
    // 正确公式：将连续值映射到 N 个等距色阶
    float levels = max(uParamFloat0, 2.0);
    float step  = levels - 1.0;
    vec3 quantized = floor(color * step + 0.5) / step;

    // ---- 边缘检测 — 基于量化后的色块边界 ----
    // 在上下左右四个方向采样，检测量化后的色块跳变
    vec2 ts = 1.0 / uResolution * uParamFloat2 * 1.5; // 描边像素偏移
    vec3 sL = floor(texture(uInputTex, vUV + vec2(-1, 0) * ts).rgb * step + 0.5) / step;
    vec3 sR = floor(texture(uInputTex, vUV + vec2( 1, 0) * ts).rgb * step + 0.5) / step;
    vec3 sU = floor(texture(uInputTex, vUV + vec2( 0,-1) * ts).rgb * step + 0.5) / step;
    vec3 sD = floor(texture(uInputTex, vUV + vec2( 0, 1) * ts).rgb * step + 0.5) / step;

    // 四方向梯度（灰度），>0 即存在色块跳变
    float grad = length(sR - sL) + length(sD - sU);

    // ---- 边缘混合 ----
    // 边缘阈值 uParamFloat1 控制描边的整体强度
    float edgeAlpha = smoothstep(0.01, uParamFloat1 * 0.5, grad);
    vec3  edgeColor = quantized * 0.25;   // 描边颜色（色块暗化）

    outColor = vec4(mix(quantized, edgeColor, edgeAlpha), 1.0);
}
