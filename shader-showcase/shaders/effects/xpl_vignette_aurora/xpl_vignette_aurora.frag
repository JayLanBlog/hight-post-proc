#version 460
// Aurora Vignette
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
float d=length(vUV-0.5)*2.0;float v=1.0-d*clamp(uParamFloat0,0,2);v=smoothstep(0.0,1.0,v);float n=sin(vUV.y*50.0+uTime*0.5)*0.02*uParamFloat1;vec3 aurora=vec3(uParamFloat2,uParamFloat3,uParamFloat4)*n;vec4 sc=texture(uInputTex,vUV);outColor=vec4(sc.rgb*v+aurora,sc.a);
}