#version 460
// Gaussian Blur — AUS Vol.15 (single-pass H+V)
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
    float blur = clamp(uParamFloat0, 0.5, 15.0);
    vec2 step = 1.0 / uResolution;
    float weights[7] = float[](0.196, 0.175, 0.132, 0.077, 0.035, 0.012, 0.003);
    vec4 colH = vec4(0);
    for (int i = -6; i <= 6; i++) {
        float w = weights[abs(i)];
        colH += texture(uInputTex, vUV + vec2(float(i)*step.x*blur, 0)) * w;
    }
    vec4 colV = vec4(0);
    for (int i = -6; i <= 6; i++) {
        float w = weights[abs(i)];
        colV += texture(uInputTex, vUV + vec2(0, float(i)*step.y*blur)) * w;
    }
    outColor = (colH + colV) * 0.5;
}
