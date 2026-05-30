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
    vec2 center = vUV - 0.5;
    float dist = length(center) * 2.0;
    vec2 dir = normalize(center + 0.0001);

    float offset = uParamFloat0 * texelSize.x;
    float radialOffset = offset * dist * uParamFloat1;
    float linearOffset = offset * (1.0 - uParamFloat1);
    float totalOffset = radialOffset + linearOffset;

    float r = texture(uInputTex, vUV + dir * totalOffset).r;
    float g = texture(uInputTex, vUV).g;
    float b = texture(uInputTex, vUV - dir * totalOffset).b;

    outColor = vec4(r, g, b, 1.0);
}
