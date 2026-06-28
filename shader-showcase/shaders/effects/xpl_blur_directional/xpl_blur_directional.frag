#version 460
// xpl_blur_directional — FULL-SCREEN strong blur, sin fade
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
    vec4 orig = texture(uInputTex, vUV);
    vec2 ps = uParamFloat0 / uResolution;
    float iter = clamp(uParamFloat1, 2.0, 40.0);
    vec4 blurred = orig;
    int n = int(iter);
    float angle = uTime * 0.4;
    vec2 dir = vec2(cos(angle), sin(angle)) * ps.x / max(float(n),1.0);
    blurred = vec4(0.0);
    for (int k = -n; k < n; k++) blurred += texture(uInputTex, vUV + dir * float(k));
    blurred /= float(2*n);
    outColor = mix(orig, blurred, 0.5+0.5*sin(uTime*1.0));
}
