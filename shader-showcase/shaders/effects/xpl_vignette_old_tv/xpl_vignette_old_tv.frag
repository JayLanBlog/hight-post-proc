#version 460
// Old TV Vignette
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
vec2 q=vUV-0.5;float d=length(q)*2.0;float v=1.0-d*clamp(uParamFloat0,0,5);v*=v;float n=(sin(vUV.y*200.0)*sin(vUV.x*220.0))*0.03*uParamFloat1;vec4 sc=texture(uInputTex,vUV);float b=dot(sc.rgb,vec3(0.299,0.587,0.114));outColor=vec4(mix(sc.rgb,vec3(b),0.7*uParamFloat2)*v+vec3(n),sc.a);
}