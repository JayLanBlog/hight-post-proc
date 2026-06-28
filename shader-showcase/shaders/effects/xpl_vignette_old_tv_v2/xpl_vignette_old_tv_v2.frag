#version 460
// Old TV Vignette V2
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
vec2 q=vUV-0.5;float d=length(q)*2.0;float v=1.0-pow(d,clamp(uParamFloat0,0.5,10.0));v=clamp(v,0,1);v*=v;float scan=sin(vUV.y*500.0+uTime*10.0)*0.5+0.5;scan=scan*0.05*uParamFloat1+0.95;vec4 sc=texture(uInputTex,vUV);float gray=dot(sc.rgb,vec3(0.299,0.587,0.114));outColor=vec4(mix(sc.rgb,vec3(gray),0.5*uParamFloat2)*v*scan,sc.a);
}