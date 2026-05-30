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

float random(vec2 st) {
    return fract(sin(dot(st, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec3 color = texture(uInputTex, vUV).rgb;
    float t = floor(uTime * uParamFloat1);

    // 随机行偏移 (扫描线撕裂)
    float lineNoise = step(0.95 - uParamFloat0 * 0.5, random(vec2(floor(vUV.y * uResolution.y / uParamFloat2), t)));
    float offset = (random(vec2(t, floor(vUV.y * 50.0))) - 0.5) * uParamFloat0 * 0.3;
    color = texture(uInputTex, vUV + vec2(offset, 0.0)).rgb;

    // RGB 通道分离
    float channelShift = step(0.98 - uParamFloat0 * 0.3, random(vec2(t, 1.0))) * uParamFloat0 * 0.05;
    color.r = texture(uInputTex, vUV + vec2(channelShift, 0.0)).r;
    color.b = texture(uInputTex, vUV - vec2(channelShift, 0.0)).b;

    // 随机色块
    float blockNoise = step(0.99 - uParamFloat0 * 0.2, random(vec2(floor(vUV.x * uResolution.x / uParamFloat2), floor(vUV.y * uResolution.y / uParamFloat2) + t)));
    color = mix(color, vec3(random(vec2(t, vUV.y * 100.0))), blockNoise);

    outColor = vec4(color, 1.0);
}
