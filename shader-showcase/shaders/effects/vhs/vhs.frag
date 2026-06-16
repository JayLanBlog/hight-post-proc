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
    // 水平跟踪失真
    float trackingLine = step(0.98 - uParamFloat3, random(vec2(floor(uTime * 10.0), 0.0)));
    float trackingOffset = trackingLine * (random(vec2(uTime, 1.0)) - 0.5) * 0.1;
    vec2 uv = vUV + vec2(trackingOffset, 0.0);

    // 色彩漂移 (Y通道延迟)
    float drift = sin(uv.y * uResolution.y * 0.5 + uTime * 2.0) * uParamFloat2;
    float r = texture(uInputTex, uv + vec2(drift, 0.0)).r;
    float g = texture(uInputTex, uv).g;
    float b = texture(uInputTex, uv - vec2(drift, 0.0)).b;
    vec3 color = vec3(r, g, b);

    // 扫描线
    float scanline = sin(uv.y * uResolution.y * 3.14159) * 0.5 + 0.5;
    color *= 1.0 - scanline * uParamFloat0;

    // 噪声
    float noise = random(vec2(uv * uResolution + uTime * 100.0));
    color += (noise - 0.5) * uParamFloat1;

    // VHS 色彩偏移 (轻微偏蓝/偏红)
    color.r *= 1.05;
    color.b *= 0.95;

    outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
