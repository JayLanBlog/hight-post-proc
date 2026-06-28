#version 460
// Lens Filter
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
vec4 c=texture(uInputTex,vUV);float v=dot(c.rgb,vec3(0.299,0.587,0.114));vec3 tint=vec3(uParamFloat0,uParamFloat1,uParamFloat2)*uParamFloat3;c.rgb=mix(c.rgb,c.rgb+tint,0.15);outColor=c;
}