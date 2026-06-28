#version 460
// xpl_blur_radial — FULL-SCREEN strong blur, sin fade
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
    float angle = uTime * 0.5;
    vec2 d = vec2(cos(angle), sin(angle)) * ps.x / float(n);
    vec2 uv = vUV;
    blurred = vec4(0.0);
    for (int i=0; i<n; i++) {
        blurred += texture(uInputTex, uv);
        uv += d;
    }
    blurred /= float(n);
    outColor = mix(orig, blurred, 0.5+0.5*sin(uTime*1.0));
}
