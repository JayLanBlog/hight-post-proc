#version 460
// Radial Motion Blur — AUS Vol.08
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
    int samples = int(clamp(uParamFloat0, 2, 20));
    float strength = clamp(uParamFloat1, 0.01, 0.3);
    vec2 center = vec2(0.5 + uParamFloat2 * 0.2, 0.5 + uParamFloat3 * 0.2);
    vec4 sum = vec4(0);
    for (int i = 0; i < 20; i++) {
        if (i >= samples) break;
        float t = float(i) / float(max(samples-1, 1));
        vec2 uv = mix(vUV, center, t * strength);
        sum += texture(uInputTex, uv);
    }
    outColor = sum / float(samples);
}
