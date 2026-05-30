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
    // Grayscale test: convert to grayscale, brightness controlled by uParamFloat0
    vec3 color = texture(uInputTex, vUV).rgb;
    float gray = dot(color, vec3(0.299, 0.587, 0.114)) * uParamFloat0;
    outColor = vec4(vec3(gray), 1.0);
}
