#version 460
// Contrast V3
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
vec4 c=texture(uInputTex,vUV);float l=dot(c.rgb,vec3(0.2126,0.7152,0.0722));vec3 bc=vec3(l)*uParamFloat0;outColor=vec4(mix(bc,c.rgb,uParamFloat1),c.a);
}