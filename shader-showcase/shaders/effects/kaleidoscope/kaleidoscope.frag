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

#define PI 3.14159265359

void main() {
    vec2 uv = (vUV - 0.5) * 2.0 / uParamFloat2;
    float angle = atan(uv.y, uv.x);
    float radius = length(uv);

    // 将角度映射到 [0, 2PI/segments]
    float segAngle = PI * 2.0 / uParamFloat0;
    angle = mod(angle + uTime * uParamFloat1, segAngle);
    if (angle > segAngle * 0.5) angle = segAngle - angle;

    // 重建UV (镜像)
    vec2 newUV = vec2(cos(angle), sin(angle)) * radius * 0.5 + 0.5;
    newUV = clamp(newUV, 0.0, 1.0);

    vec3 color = texture(uInputTex, newUV).rgb;
    outColor = vec4(color, 1.0);
}
