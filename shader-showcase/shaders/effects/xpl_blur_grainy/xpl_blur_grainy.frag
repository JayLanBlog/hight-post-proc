#version 460
// xpl_blur_grainy — FULL-SCREEN strong blur, sin fade
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
    blurred = vec4(0.0);
    float r = fract(sin(dot(vUV+uTime*0.17, vec2(1233.224,1743.335))));
    for (int k=0; k<n; k++) {
        r = fract(43758.5453*r + 0.61432);
        float rx = (r-0.5)*2.0*ps.x;
        r = fract(43758.5453*r + 0.61432);
        float ry = (r-0.5)*2.0*ps.y;
        blurred += texture(uInputTex, vUV+vec2(rx,ry));
    }
    blurred /= float(n);
    outColor = mix(orig, blurred, 0.5+0.5*sin(uTime*1.0));
}
