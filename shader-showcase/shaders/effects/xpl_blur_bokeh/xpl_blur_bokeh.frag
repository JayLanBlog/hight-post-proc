#version 460
// xpl_blur_bokeh — FULL-SCREEN strong blur, sin fade
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
    float golden = 2.399963 + uTime*0.2;
    mat2 rot = mat2(cos(golden), sin(golden), -sin(golden), cos(golden));
    vec4 acc = vec4(0.0), div = vec4(0.0);
    float fr = 1.0;
    vec2 ang = vec2(0.0, ps.x);
    for (int j=0; j<n; j++) {
        fr += 1.0/fr;
        ang = rot*ang;
        vec4 s = texture(uInputTex, vUV+(fr-1.0)*ang);
        acc += s*s;
        div += s;
    }
    blurred = acc/max(div, vec4(0.0001));
    outColor = mix(orig, blurred, 0.5+0.5*sin(uTime*1.0));
}
