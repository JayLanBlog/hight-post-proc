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
    vec2 texelSize = 1.0 / uResolution;
    float rad = radians(uParamFloat1);
    vec2 dir = vec2(cos(rad), sin(rad)) * texelSize;

    vec3 color = texture(uInputTex, vUV).rgb;
    vec3 neighbor = texture(uInputTex, vUV + dir).rgb;
    vec3 diff = (neighbor - color) * uParamFloat0 + 0.5;
    outColor = vec4(clamp(diff, 0.0, 1.0), 1.0);
}
