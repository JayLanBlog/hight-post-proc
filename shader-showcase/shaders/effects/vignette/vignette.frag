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
    vec2 uv = vUV - 0.5;
    // 宽高比校正
    float aspect = uResolution.x / uResolution.y;
    uv.x *= mix(1.0, aspect, uParamFloat2);
    float dist = length(uv);
    float vignette = smoothstep(0.5 - uParamFloat1 * 0.5, 0.5 + uParamFloat1 * 0.2, dist * (1.0 + uParamFloat0));
    color *= 1.0 - vignette;
    outColor = vec4(color, 1.0);
}
