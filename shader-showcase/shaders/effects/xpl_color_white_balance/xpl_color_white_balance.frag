#version 460
// White Balance
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
vec4 c=texture(uInputTex,vUV);float t=uParamFloat0;vec3 wb=c.rgb;wb.r=clamp(wb.r+(t>0?t*0.1:t*0.05),0,1);wb.b=clamp(wb.b+(t<0?-t*0.1:-t*0.05),0,1);outColor=vec4(mix(c.rgb,wb,uParamFloat1),c.a);
}