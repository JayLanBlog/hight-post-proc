#version 460
// Sharpen V3
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
vec2 ps=1.0/uResolution;vec2 center=vUV;vec2 dir=center-0.5;float d=length(dir)*2.0;float f=mix(uParamFloat0,uParamFloat1,d);vec4 blur=texture(uInputTex,center+vec2(-ps.x,-ps.y))*0.25+texture(uInputTex,center+vec2(ps.x,-ps.y))*0.25+texture(uInputTex,center+vec2(-ps.x,ps.y))*0.25+texture(uInputTex,center+vec2(ps.x,ps.y))*0.25;vec4 sc=texture(uInputTex,center);outColor=sc+(sc-blur)*f;
}