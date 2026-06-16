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
    vec2 uv = (vUV - 0.5) * 2.0; // [-1, 1]

    // 径向畸变 (Brown-Conrady 模型简化版)
    float r2 = dot(uv, uv);
    float r4 = r2 * r2;
    float distortion = 1.0 + uParamFloat0 * r2 + uParamFloat0 * 0.5 * r4;
    vec2 distortedUV = uv * distortion / uParamFloat1;
    distortedUV = distortedUV * 0.5 + 0.5;

    // 超出范围采样最近边缘
    distortedUV = clamp(distortedUV, 0.0, 1.0);

    vec3 color = texture(uInputTex, distortedUV).rgb;
    outColor = vec4(color, 1.0);
}
