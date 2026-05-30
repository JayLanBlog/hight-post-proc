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

    // 颜色量化
    float levels = max(uParamFloat0, 2.0);
    vec3 quantized = floor(color * levels) / max(levels - 1.0, 1.0);
    quantized = clamp(quantized, 0.0, 1.0);

    // 边缘检测 — 量化色块边界
    vec2 ts = 1.0 / uResolution * uParamFloat2;
    vec3 nr = clamp(floor(texture(uInputTex, vUV + vec2( 1, 0) * ts).rgb * levels) / max(levels - 1.0, 1.0), 0.0, 1.0);
    vec3 nd = clamp(floor(texture(uInputTex, vUV + vec2( 0, 1) * ts).rgb * levels) / max(levels - 1.0, 1.0), 0.0, 1.0);
    float isEdge = any(notEqual(quantized, nr)) || any(notEqual(quantized, nd)) ? 1.0 : 0.0;

    // 边缘变暗
    float outline = clamp(uParamFloat1 * 4.0, 0.0, 1.0);
    color = mix(quantized, quantized * 0.15, isEdge * outline);

    outColor = vec4(color, 1.0);
}
