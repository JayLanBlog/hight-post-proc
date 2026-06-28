#version 460
// Color Replace
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
vec4 c=texture(uInputTex,vUV);vec3 from=vec3(uParamFloat0,uParamFloat1,uParamFloat2);vec3 to=vec3(uParamFloat3,uParamFloat4,uParamFloat5);float d=length(c.rgb-from);float r=0.3;outColor=vec4(mix(c.rgb,to,clamp(1.0-d/r,0,1)),c.a);
}