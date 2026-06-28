#version 460
// Tint
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
vec4 c=texture(uInputTex,vUV);vec3 tint=vec3(uParamFloat0,uParamFloat1,uParamFloat2);outColor=vec4(mix(c.rgb,c.rgb*tint,uParamFloat3),c.a);
}