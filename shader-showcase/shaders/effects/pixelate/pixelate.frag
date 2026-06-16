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
    vec2 pixelCount = uResolution / max(uParamFloat0, 1.0);
    // Sample at block center to avoid edge bleeding with linear filtering
    vec2 uv = (floor(vUV * pixelCount) + 0.5) / pixelCount;
    vec3 color = texture(uInputTex, uv).rgb;
    outColor = vec4(color, 1.0);
}
