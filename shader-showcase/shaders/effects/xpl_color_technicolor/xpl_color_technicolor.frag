#version 460
// Technicolor
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
vec4 c=texture(uInputTex,vUV);float a=uParamFloat0;vec3 r=clamp(c.rgb-vec3(0.5)*a,0,1);vec3 g=clamp(c.rgb+vec3(0.5)*a,0,1);vec3 b=clamp(c.rgb+vec3(0.25)*a,0,1);float p=uParamFloat1;outColor=vec4(mix(c.rgb,mix(r,mix(g,b,0.5),0.5),p),c.a);
}